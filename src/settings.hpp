#pragma once
#include "player.hpp"
#pragma once
#include "chunks.hpp"

struct GameSettings {
    float mouseSensitivity = 0.002f;
    int renderDistance = 16;
    float sfxVolume = 1.0f;
    float musicVolume = 1.0f;
    float fov = 90.0f;
};

GameSettings Settings;

void SaveSettings() {
    FILE* f = fopen("settings.json", "w");
    if (!f) return;
    fprintf(f, "{\n");
    fprintf(f, "    \"mouseSensitivity\": %.4f,\n", Settings.mouseSensitivity);
    fprintf(f, "    \"renderDistance\": %d,\n", Settings.renderDistance);
    fprintf(f, "    \"sfxVolume\": %.4f,\n", Settings.sfxVolume);
    fprintf(f, "    \"musicVolume\": %.4f,\n", Settings.musicVolume);
    fprintf(f, "    \"fov\": %.4f\n", Settings.fov);
    fprintf(f, "}\n");
    fclose(f);
}

void LoadSettings() {
    FILE* f = fopen("settings.json", "r");
    if (!f) return; // use defaults if no file
    fscanf(f, " { \"mouseSensitivity\": %f , \"renderDistance\": %d , \"sfxVolume\": %f , \"musicVolume\": %f , \"fov\": %f }",
        &Settings.mouseSensitivity,
        &Settings.renderDistance,
        &Settings.sfxVolume,
        &Settings.musicVolume,
        &Settings.fov);
    fclose(f);
}

int DrawPauseMenu(bool& paused, bool& settingsOpen, std::vector<Chunk> Chunks, Player player, float DayTime,int survival) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    
    DrawRectangle(0, 0, sw, sh, {0,0,0,120});
    
    float bw = 300, bh = 45, bx = sw/2 - 150;
    Color stone = {120,120,120,255};
    Color stoneHover = {160,160,160,255};
    
    DrawText("Paused", sw/2 - MeasureText("Paused", 30)/2, sh*0.2f, 30, WHITE);
    
    if (DrawButton("Resume", {bx, sh*0.35f, bw, bh}, stone, stoneHover, WHITE))
        paused = false;
    if (DrawButton("Settings", {bx, sh*0.35f + 55, bw, bh}, stone, stoneHover, WHITE))
        settingsOpen = true;
    if (DrawButton("Save & Quit", {bx, sh*0.35f + 110, bw, bh}, stone, stoneHover, WHITE)) {
        SaveWorld(Chunks, player, DayTime,survival);
        return 1;
    }
    return 0;
}

bool DrawSlider(const char* label, float& value, float min, float max, float x, float y, float w) {
    int sh = GetScreenHeight();
    DrawText(label, x, y, 18, WHITE);
    
    float trackY = y + 25;
    DrawRectangle(x, trackY, w, 16, DARKGRAY);
    
    float t = (value - min) / (max - min);
    float handleX = x + t * w;
    DrawRectangle(handleX, trackY, 10, 16, WHITE);
    
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (mouse.x >= x && mouse.x <= x + w &&
            mouse.y >= trackY - 10 && mouse.y <= trackY + 16) {
            value = min + ((mouse.x - x) / w) * (max - min);
            value = Clamp(value, min, max);
            return true;
        }
    }
    DrawText(TextFormat("%.2f", value), x + w + 10, y + 20, 16, GRAY);
    return false;
}

void DrawSettingsMenu(bool& settingsOpen, Music Song) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    
    DrawRectangle(0, 0, sw, sh, {0,0,0,180});
    DrawText("Settings", sw/2 - MeasureText("Settings", 30)/2, 30, 30, WHITE);
    
    float bx = sw/2 - 200.0f;
    bool changed = false;
    
    //changed |= DrawSlider("Mouse Sensitivity", Settings.mouseSensitivity, 0.0005f, 0.005f, bx, 100, 400);
    changed |= DrawSlider("FOV", Settings.fov, 60.0f, 120.0f, bx, 160, 400);
    changed |= DrawSlider("Music Volume", Settings.musicVolume, 0.0f, 1.0f, bx, 220, 400);
    changed |= DrawSlider("SFX Volume", Settings.sfxVolume, 0.0f, 1.0f, bx, 280, 400);
    
    DrawText(TextFormat("Render Distance: %d", Settings.renderDistance), bx, 340, 18, WHITE);
    if (DrawButton("-", {bx, 365, 40, 35}, {80,80,80,255}, {120,120,120,255}, WHITE))
        Settings.renderDistance = max(4, Settings.renderDistance - 4);
    if (DrawButton("+", {bx + 50, 365, 40, 35}, {80,80,80,255}, {120,120,120,255}, WHITE))
        Settings.renderDistance = min(32, Settings.renderDistance + 4);
    
    if (changed) {
        SetMusicVolume(Song, Settings.musicVolume);
        SaveSettings();
    }
    
    if (DrawButton("Back", {bx, (float)sh - 60, 150, 40}, {80,80,80,255}, {120,120,120,255}, WHITE)) {
        settingsOpen = false;
        SaveSettings();
    }
}