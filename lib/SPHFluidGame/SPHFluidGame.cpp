#pragma once

#include <vector>
#include <math.h>
#include <Arduino.h>       // For random(), etc., on Arduino/ESP
#include "SSD1306Wire.h"   // Or your specific SSD1306 library

class SPHFluidGame {
public:
    struct Particle {
        float x, y;
        float vx, vy;
        float density;
        float pressure;
    };

    SPHFluidGame(SSD1306Wire& disp)
        : display(disp)
    {
        // Initialize simulation parameters
        /*
        Particle Count
        -------------
        SPH is O(N²). For each update, every particle checks every other particle.
        Keep numParticles small (50–200) if you want a usable frame rate on ESP/Arduino.

        Smoothing Length (smoothingLength)
        ----------------------------------
        Controls the neighborhood size for interactions. If it’s too big, everything 
        interacts too strongly. If it’s too small, you’ll get low density or super 
        local collisions.

        Stiffness (stiffness)
        ---------------------
        Higher values make the fluid act more like an incompressible liquid but 
        can cause instability.

        Viscosity
        ---------
        Smooths out velocity differences. If your fluid is “exploding” or jiggling 
        too much, bump this up.

        Time Step (dt)
        --------------
        Must be small enough to keep the simulation stable. If you see instability or 
        infinite velocities, reduce dt.

        Gravity vs. IMU
        ---------------
        In the example, we just add accelX * 5.0f or so to simulate tilt. You’ll need 
        to tweak that scaling. If you want actual gravity in the real sense, keep 
        gravityY = 9.8f; or set it lower for more subtle fluid.

        Performance
        -----------
        With naive O(N²) loops, 200 particles → 40,000 interactions each frame. That 
        might be near the limit on certain boards. You can optimize with cell grids 
        or neighbor searches if needed, but that’s more complex.

        Debug
        -----
        If you see positions/velocities go to infinity, your parameters (stiffness, 
        dt, smoothing length, etc.) might be out of balance or your IMU feed is 
        returning very large values.
        */

        numParticles = 200;         // Try small first
        smoothingLength = 5.0f;     // h
        restDensity    = 1.0f;      // ρ0
        stiffness      = 2.0f;      // k
        viscosity      = 0.8f;      // μ
        gravityX       = 0.0f;      // No horizontal gravity
        gravityY       = 9.8f;      // Earth-like gravity
        damping        = 0.99f;     // For velocity damping at boundaries
        dt             = 0.1f;      // Time step
        screenWidth    = 128;
        screenHeight   = 64;

        resetParticles();
    }

    //--------------------------------------------------------------------------------
    // Step the simulation with optional external acceleration (e.g., from IMU)
    //--------------------------------------------------------------------------------
    void update(float accelX, float accelY) {
        // Combine “gravity-like” acceleration with IMU tilt
        // (Tune scaling as needed)
        float ax = gravityX + accelX * -5.0f;
        float ay = gravityY + accelY * 5.0f;

        // 1) Compute density and pressure
        computeDensityPressure();

        // 2) Compute forces
        computeForces(ax, ay);

        // 3) Integrate (update positions and velocities)
        integrate();

        // 4) Render on the OLED
        render();
    }

    //--------------------------------------------------------------------------------
    // Re-randomize or reset the particle positions
    //--------------------------------------------------------------------------------
    void resetParticles() {
        particles.clear();
        particles.reserve(numParticles);

        for (int i = 0; i < numParticles; i++) {
            Particle p;
            p.x       = random(screenWidth);
            p.y       = random(screenHeight);
            p.vx      = 0.0f;
            p.vy      = 0.0f;
            p.density = restDensity;
            p.pressure = 0.0f;
            particles.push_back(p);
        }
    }

private:
    // ---- SPH parameters ----
    int   numParticles;
    float smoothingLength;  // h
    float restDensity;      // ρ0
    float stiffness;        // k
    float viscosity;        // μ
    float gravityX;         // external accel in X
    float gravityY;         // external accel in Y
    float damping;          // boundary damping
    float dt;               // timestep

    // ---- Display / screen ----
    SSD1306Wire& display;
    int screenWidth;
    int screenHeight;

    // ---- Particle container ----
    std::vector<Particle> particles;

    //--------------------------------------------------------------------------------
    // Compute densities & pressures
    //--------------------------------------------------------------------------------
    void computeDensityPressure() {
        float h2     = smoothingLength * smoothingLength;
        float mass    = 1.0f;  // Not physically accurate, but consistent for all
        float poly6K  = 315.0f / (64.0f * (float)M_PI * powf(smoothingLength, 9));

        // For each particle i
        for (int i = 0; i < numParticles; i++) {
            Particle &pi = particles[i];

            // Reset density
            pi.density = 0.0f;

            // Sum over neighbors j (naive O(N^2))
            for (int j = 0; j < numParticles; j++) {
                float dx = pi.x - particles[j].x;
                float dy = pi.y - particles[j].y;
                float r2 = dx*dx + dy*dy;

                if (r2 < h2) {
                    // Poly6 kernel for density
                    float t = (h2 - r2);
                    pi.density += mass * poly6K * t * t * t;
                }
            }

            // Pressure (simple Tait equation or linear eqn)
            pi.pressure = stiffness * (pi.density - restDensity);
            if (pi.pressure < 0.0f) {
                pi.pressure = 0.0f; // No negative pressure in this simple approach
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Compute forces from pressure & viscosity & external acceleration
    //--------------------------------------------------------------------------------
    void computeForces(float ax, float ay) {
        float h      = smoothingLength;
        float h2     = h * h;
        float mass   = 1.0f;
        float spikyK = -45.0f / ((float)M_PI * powf(h, 6));     // Spiky kernel grad
        float viscoK = 45.0f  / ((float)M_PI * powf(h, 6));     // Viscosity kernel laplacian

        // For each particle i
        for (int i = 0; i < numParticles; i++) {
            Particle &pi = particles[i];

            // Start with external acceleration (gravity + IMU tilt)
            float fx = 0.0f;
            float fy = 0.0f;

            // naive O(N^2) loop
            for (int j = 0; j < numParticles; j++) {
                if (i == j) continue;

                Particle &pj = particles[j];

                float dx = pi.x - pj.x;
                float dy = pi.y - pj.y;
                float r2 = dx*dx + dy*dy;

                if (r2 < h2 && r2 > 0.000001f) {
                    float r  = sqrtf(r2);
                    float invR = 1.0f / r;

                    // Pressure force
                    float term = (pi.pressure + pj.pressure) / (2.0f * pj.density);
                    float spiky = spikyK * powf(h - r, 2);

                    fx += mass * term * spiky * dx * invR;
                    fy += mass * term * spiky * dy * invR;

                    // Viscosity force
                    float vijx = pj.vx - pi.vx;
                    float vijy = pj.vy - pi.vy;
                    float visc = viscoK * (h - r);

                    fx += viscosity * vijx * visc;
                    fy += viscosity * vijy * visc;
                }
            }

            // Add external accel as force: F = m * a
            fx += ax * pi.density;  // approximate mass ~ density
            fy += ay * pi.density;

            // Convert to acceleration
            float invDen = 1.0f / pi.density;
            pi.vx += (fx * invDen) * dt;
            pi.vy += (fy * invDen) * dt;
        }
    }

    //--------------------------------------------------------------------------------
    // Integrate and apply boundary conditions
    //--------------------------------------------------------------------------------
    void integrate() {
        for (int i = 0; i < numParticles; i++) {
            Particle &p = particles[i];

            // Update positions
            p.x += p.vx * dt;
            p.y += p.vy * dt;

            // Bounce at boundaries
            if (p.x < 0) {
                p.x = 0; 
                p.vx *= -damping;
            } else if (p.x >= screenWidth) {
                p.x = screenWidth - 1;
                p.vx *= -damping;
            }

            if (p.y < 0) {
                p.y = 0;
                p.vy *= -damping;
            } else if (p.y >= screenHeight) {
                p.y = screenHeight - 1;
                p.vy *= -damping;
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Render particles
    //--------------------------------------------------------------------------------
    void render() {
        display.clear();

        // If needed in your library, set draw color:
        // display.setColor(WHITE);

        for (int i = 0; i < numParticles; i++) {
            int px = static_cast<int>(particles[i].x);
            int py = static_cast<int>(particles[i].y);

            if (px >= 0 && px < screenWidth && py >= 0 && py < screenHeight) {
                // If your lib only provides setPixel, use that
                display.setPixel(px, py);
            }
        }

        display.display();
    }
};
