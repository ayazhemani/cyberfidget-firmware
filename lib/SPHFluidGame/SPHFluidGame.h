#pragma once

#include <vector>
#include <math.h>
#include <Arduino.h>
#include "DisplayProxy.h" 

class SPHFluidGame {
public:
    struct Particle {
        float x, y;
        float vx, vy;
        float density;
        float pressure;
    };

    SPHFluidGame();

    // Updates the simulation by one step, taking external acceleration (e.g., from an IMU).
    void update();

    // Resets/re-randomizes particles.
    void resetParticles();

    // Adjusts the number of particles rendered
    void setParticleCount(int newCount);

private:
    // ---- SPH parameters ----
    int   numParticles;       // e.g., 100 or 200 for microcontroller
    float smoothingLength;    // h
    float restDensity;        // ρ0
    float stiffness;          // k
    float viscosity;          // μ
    float gravityX;           // external accel in X
    float gravityY;           // external accel in Y
    float damping;            // boundary damping factor
    float dt;                 // timestep
    float particleRadius;     // e.g. 2.0, how big the particle is on screen

    // -- Surface Tension (cohesion) parameter --
    float cohesionStrength;   // Additional force pulling particles together

    // ---- Display / screen ----
    DisplayProxy& display;
    int screenWidth;
    int screenHeight;

    // ---- Particle container ----
    std::vector<Particle> particles;

    // Core SPH steps
    void computeDensityPressure();
    void computeForces(float ax, float ay);
    void resolveParticleCollisions();
    void integrate();
    void render();
};
