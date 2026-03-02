#include "ID3Scanner.h"
#include <algorithm>

// ---------------------------------------------------------------------------
// TrackInfo
// ---------------------------------------------------------------------------
void TrackInfo::computeDisplay() {
    if (artist.length() && title.length()) {
        display = artist + " - " + title;
    } else if (title.length()) {
        display = title;
    } else {
        // Strip path and extension from filename
        String s = path;
        int lastSlash = s.lastIndexOf('/');
        if (lastSlash >= 0) s = s.substring(lastSlash + 1);
        int lastDot = s.lastIndexOf('.');
        if (lastDot >= 0) s = s.substring(0, lastDot);
        display = s;
    }
}

// ---------------------------------------------------------------------------
// ID3v2 syncsafe integer (4 bytes, 7 bits each)
// ---------------------------------------------------------------------------
static uint32_t readSyncsafe(const uint8_t* b) {
    return ((uint32_t)b[0] << 21) |
           ((uint32_t)b[1] << 14) |
           ((uint32_t)b[2] << 7)  |
           (uint32_t)b[3];
}

static uint32_t readBE32(const uint8_t* b) {
    return ((uint32_t)b[0] << 24) |
           ((uint32_t)b[1] << 16) |
           ((uint32_t)b[2] << 8)  |
           (uint32_t)b[3];
}

// Extract text from ID3 frame data (handles encoding byte)
static String extractText(const uint8_t* data, int len) {
    if (len < 2) return "";
    uint8_t encoding = data[0];
    const uint8_t* text = data + 1;
    int textLen = len - 1;

    if (encoding == 0 || encoding == 3) {
        // ISO-8859-1 or UTF-8 — treat as plain ASCII/UTF-8
        char buf[256];
        int copyLen = textLen > 255 ? 255 : textLen;
        memcpy(buf, text, copyLen);
        buf[copyLen] = '\0';
        // Trim trailing nulls/spaces
        while (copyLen > 0 && (buf[copyLen-1] == '\0' || buf[copyLen-1] == ' '))
            buf[--copyLen] = '\0';
        return String(buf);
    }

    if (encoding == 1 || encoding == 2) {
        // UTF-16 — extract ASCII characters (skip BOM if present)
        int start = 0;
        if (encoding == 1 && textLen >= 2) {
            // Skip BOM (FF FE or FE FF)
            if ((text[0] == 0xFF && text[1] == 0xFE) ||
                (text[0] == 0xFE && text[1] == 0xFF)) {
                start = 2;
            }
        }
        bool littleEndian = (encoding == 1 && start >= 2 && text[0] == 0xFF);
        char buf[256];
        int outIdx = 0;
        for (int i = start; i + 1 < textLen && outIdx < 255; i += 2) {
            char c = littleEndian ? text[i] : text[i+1];
            if (c == '\0') break;
            buf[outIdx++] = c;
        }
        buf[outIdx] = '\0';
        return String(buf);
    }

    return "";
}

// ---------------------------------------------------------------------------
// Scan a single MP3 file for ID3 tags
// ---------------------------------------------------------------------------
bool ID3Scanner::scanFile(const char* path, String& title, String& artist) {
    File f = SD.open(path, FILE_READ);
    if (!f) return false;

    title = "";
    artist = "";
    bool found = false;

    // --- Try ID3v2 (at file start) ---
    uint8_t header[10];
    if (f.read(header, 10) == 10 &&
        header[0] == 'I' && header[1] == 'D' && header[2] == '3') {

        uint8_t versionMajor = header[3];
        uint32_t tagSize = readSyncsafe(header + 6);

        // Limit read to 8KB to avoid slow scans on huge tags
        if (tagSize > 8192) tagSize = 8192;

        uint32_t pos = 0;
        while (pos + 10 < tagSize) {
            uint8_t frameHeader[10];
            if (f.read(frameHeader, 10) != 10) break;
            pos += 10;

            char frameId[5] = {0};
            memcpy(frameId, frameHeader, 4);

            // Check for padding (all zeros)
            if (frameId[0] == '\0') break;

            uint32_t frameSize;
            if (versionMajor >= 4) {
                frameSize = readSyncsafe(frameHeader + 4);
            } else {
                frameSize = readBE32(frameHeader + 4);
            }

            if (frameSize == 0 || frameSize > tagSize - pos) break;

            bool isTitle  = (strcmp(frameId, "TIT2") == 0);
            bool isArtist = (strcmp(frameId, "TPE1") == 0);

            if (isTitle || isArtist) {
                int readSize = frameSize > 512 ? 512 : frameSize;
                uint8_t* data = (uint8_t*)malloc(readSize);
                if (data) {
                    if (f.read(data, readSize) == readSize) {
                        String text = extractText(data, readSize);
                        if (isTitle) title = text;
                        else artist = text;
                        found = true;
                    }
                    free(data);
                }
                // Skip remainder if frame was larger than our read
                if ((int)frameSize > readSize) {
                    f.seek(f.position() + (frameSize - readSize));
                }
            } else {
                // Skip frame data
                f.seek(f.position() + frameSize);
            }
            pos += frameSize;

            if (title.length() && artist.length()) break;
        }
    }

    // --- Try ID3v1 (at file end) if we're still missing data ---
    if (!title.length() || !artist.length()) {
        size_t fileSize = f.size();
        if (fileSize > 128) {
            f.seek(fileSize - 128);
            uint8_t tag[128];
            if (f.read(tag, 128) == 128 &&
                tag[0] == 'T' && tag[1] == 'A' && tag[2] == 'G') {

                // ID3v1: title at offset 3 (30 bytes), artist at 33 (30 bytes)
                auto trimField = [](const uint8_t* src, int maxLen) -> String {
                    char buf[31];
                    int len = maxLen;
                    memcpy(buf, src, len);
                    buf[len] = '\0';
                    // Trim trailing spaces and nulls
                    while (len > 0 && (buf[len-1] == ' ' || buf[len-1] == '\0'))
                        buf[--len] = '\0';
                    return String(buf);
                };

                if (!title.length()) {
                    title = trimField(tag + 3, 30);
                }
                if (!artist.length()) {
                    artist = trimField(tag + 33, 30);
                }
                if (title.length() || artist.length()) found = true;
            }
        }
    }

    f.close();
    return found;
}

// ---------------------------------------------------------------------------
// Scan all MP3 files in a directory
// ---------------------------------------------------------------------------
void ID3Scanner::scanAllFiles(const char* rootDir, const char* ext,
                              std::vector<TrackInfo>& out,
                              void (*progressCb)(int, int)) {
    out.clear();

    // Enumerate files using SD directory listing
    File root = SD.open(rootDir);
    if (!root || !root.isDirectory()) return;

    // Collect all matching filenames first
    std::vector<String> filePaths;
    File entry;
    while ((entry = root.openNextFile())) {
        if (!entry.isDirectory()) {
            String name = entry.name();
            // Check extension
            String lower = name;
            lower.toLowerCase();
            if (lower.endsWith(String(".") + ext)) {
                // Ensure full path
                String fullPath = name;
                if (!fullPath.startsWith("/")) fullPath = String("/") + fullPath;
                filePaths.push_back(fullPath);
            }
        }
        entry.close();
    }
    root.close();

    int total = filePaths.size();

    for (int i = 0; i < total; i++) {
        if (progressCb) progressCb(i, total);

        TrackInfo track;
        track.path = filePaths[i];

        String t, a;
        scanFile(track.path.c_str(), t, a);
        track.title = t;
        track.artist = a;
        track.computeDisplay();

        out.push_back(track);
    }

    // Sort by display name
    std::sort(out.begin(), out.end(), [](const TrackInfo& a, const TrackInfo& b) {
        return a.display < b.display;
    });

    if (progressCb) progressCb(total, total);
}

// ---------------------------------------------------------------------------
// Cache management
// ---------------------------------------------------------------------------
bool ID3Scanner::isCacheValid(const char* cachePath, int expectedFileCount) {
    File f = SD.open(cachePath, FILE_READ);
    if (!f) return false;

    String firstLine = f.readStringUntil('\n');
    f.close();

    int cachedCount = firstLine.toInt();
    return (cachedCount == expectedFileCount && expectedFileCount > 0);
}

bool ID3Scanner::loadCache(const char* cachePath, std::vector<TrackInfo>& out) {
    File f = SD.open(cachePath, FILE_READ);
    if (!f) return false;

    out.clear();

    // First line is file count
    String countLine = f.readStringUntil('\n');
    int count = countLine.toInt();
    if (count <= 0) { f.close(); return false; }

    out.reserve(count);

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        // Format: path|title|artist
        int sep1 = line.indexOf('|');
        if (sep1 < 0) continue;
        int sep2 = line.indexOf('|', sep1 + 1);
        if (sep2 < 0) continue;

        TrackInfo track;
        track.path = line.substring(0, sep1);
        track.title = line.substring(sep1 + 1, sep2);
        track.artist = line.substring(sep2 + 1);
        track.computeDisplay();

        out.push_back(track);
    }

    f.close();
    return !out.empty();
}

bool ID3Scanner::saveCache(const char* cachePath, const std::vector<TrackInfo>& tracks) {
    File f = SD.open(cachePath, FILE_WRITE);
    if (!f) return false;

    f.println(tracks.size());

    for (const auto& t : tracks) {
        f.print(t.path);
        f.print('|');
        f.print(t.title);
        f.print('|');
        f.println(t.artist);
    }

    f.close();
    return true;
}
