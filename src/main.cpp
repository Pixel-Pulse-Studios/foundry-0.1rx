//Default C++ Libs

#include <vector>

#include <cstdint>

#include <cmath>

#include <cstring>

#include <string>

#include <cstdio>

#include <queue>

#include <sys/stat.h>

#include <iterator>

#include <ctime>

#include <sstream>

#include <iomanip>

using namespace std;

// Out House
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX
#include <windows.h>

#include "kissnet.hpp"

//Raylib Stuffs


#include "raylib.h"

#include "raymath.h"

#include "rlgl.h"

//InHouse Imports

#include "chunks.hpp"

#include "inventory.hpp"

#include "mesh.hpp"

#include "textures.hpp"

#include "player.hpp"

#include "save.hpp"

#include "titlescreen.hpp"

#include "settings.hpp"

int screenWidth = 800;
int screenHeight = 600;

void PlaySFX(Sound sound) {
    SetSoundVolume(sound, Settings.sfxVolume);
    PlaySound(sound);
}

void PlayerBuild(Player player, vector<Chunk>& chunks, int& block, float DayTime = 0.0f) {
    Vector3 rayforward = {
        cosf(player.Pitch) * sinf(player.Yaw),
        sinf(player.Pitch),
        cosf(player.Pitch) * cosf(player.Yaw)
    };
    
    Vector3 RayPos = {
        player.Position.x,
        player.Position.y + player.Size.y - 0.2f,
        player.Position.z
    };
    float RaySpeed = 0.1f;
        
    if (block > (int)BlockEntries.size()-1) block = 1;
    if (block < 1) block = BlockEntries.size()-1;
    
    // Break Block
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 0; i < 10 / RaySpeed; i++) {
            RayPos = Vector3Add(RayPos, Vector3Scale(rayforward, RaySpeed));
            if (BlockAtPos(RayPos, chunks) && BlockEntries[BlockAtPos(RayPos, chunks)].solid) {
                int hitBlock = BlockAtPos(RayPos, chunks);
                SetBlockAtPos(RayPos, chunks, 0, DayTime);
                Sound breakSounds[] = {(Sound){0}, SFX_BreakHard, SFX_BreakGrainy, SFX_BreakSoft};
                if (hitBlock > 0 && hitBlock < (int)BlockEntries.size())
                    PlaySFX(breakSounds[BlockEntries[hitBlock].Soundtype]);
                break;
            }
        }
    }
    
    // Place Block
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        Vector3 lastRayPos = RayPos;
        for (int i = 0; i < 10 / RaySpeed; i++) {
            lastRayPos = RayPos;
            RayPos = Vector3Add(RayPos, Vector3Scale(rayforward, RaySpeed));
            if (BlockAtPos(RayPos, chunks) && BlockEntries[BlockAtPos(RayPos, chunks)].solid) {
                Vector3 blockPos = {floorf(lastRayPos.x), floorf(lastRayPos.y), floorf(lastRayPos.z)};
                Vector3 playerMin = {player.Position.x - 0.5f, player.Position.y, player.Position.z - 0.5f};
                Vector3 playerMax = {player.Position.x + 0.5f, player.Position.y + player.Size.y, player.Position.z + 0.5f};
                bool insidePlayer = blockPos.x >= playerMin.x && blockPos.x < playerMax.x &&
                                    blockPos.y >= playerMin.y && blockPos.y < playerMax.y &&
                                    blockPos.z >= playerMin.z && blockPos.z < playerMax.z;
                if (!insidePlayer) {
                    SetBlockAtPos(lastRayPos, chunks, block, DayTime);
                    Sound placeSounds[] = {(Sound){0}, SFX_PlaceHard, SFX_PlaceGrainy, SFX_PlaceSoft};
                    if (block > 0 && block < (int)BlockEntries.size())
                        PlaySFX(placeSounds[BlockEntries[block].Soundtype]);
                }
                break;
            }
        }
    }
}

Color LerpColor(Color a, Color b, float t) {
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}

Color GetSkyColor(float dayTime) {
    Color night = {10, 10, 30, 255};
    Color sunrise = {255, 120, 80, 255};
    Color day = SKYBLUE;
    if (dayTime < 0.25f) return LerpColor(night, sunrise, dayTime / 0.25f);
    else if (dayTime < 0.5f) return LerpColor(sunrise, day, (dayTime - 0.25f) / 0.25f);
    else if (dayTime < 0.75f) return LerpColor(day, sunrise, (dayTime - 0.5f) / 0.25f);
    else return LerpColor(sunrise, night, (dayTime - 0.75f) / 0.25f);
}

void GenerateBlockPreviews() {
    Camera3D previewCam = {0};
    previewCam.position = {1.5f, 1.5f, 1.5f};
    previewCam.target = {0.0f, 0.0f, 0.0f};
    previewCam.up = {0.0f, 1.0f, 0.0f};
    previewCam.fovy = 45.0f;
    previewCam.projection = CAMERA_PERSPECTIVE;

    for (int i = 0; i < (int)BlockEntries.size(); i++) {
        RenderTexture2D preview = LoadRenderTexture(50, 50);
        
        float tileSize = 128.0f / 8.0f;
        int tileID = BlockEntries[i].TextureSrc[0];
        int sideTileID = BlockEntries[i].TextureSrc[1] ? BlockEntries[i].TextureSrc[1] : tileID;
        
        auto getUV = [&](int tile) -> Rectangle {
            int tx = tile % 8;
            int ty = tile / 8;
            return {tx * tileSize, ty * tileSize, tileSize, tileSize};
        };
        
        Rectangle topUV  = getUV(tileID);
        Rectangle sideUV = getUV(sideTileID);

        BeginTextureMode(preview);
            ClearBackground(BLANK);
            BeginMode3D(previewCam);
                rlSetTexture(GlobalAtlas.id);
                rlBegin(RL_QUADS);
                    // TOP face
                    rlTexCoord2f(topUV.x / 128.0f, topUV.y / 128.0f);
                    rlVertex3f(-0.5f, 0.5f, 0.5f);
                    rlTexCoord2f((topUV.x + topUV.width) / 128.0f, topUV.y / 128.0f);
                    rlVertex3f(0.5f, 0.5f, 0.5f);
                    rlTexCoord2f((topUV.x + topUV.width) / 128.0f, (topUV.y + topUV.height) / 128.0f);
                    rlVertex3f(0.5f, 0.5f, -0.5f);
                    rlTexCoord2f(topUV.x / 128.0f, (topUV.y + topUV.height) / 128.0f);
                    rlVertex3f(-0.5f, 0.5f, -0.5f);

                    // FRONT face
                    rlTexCoord2f(sideUV.x/128.0f, (sideUV.y+sideUV.height)/128.0f);
                    rlVertex3f(-0.5f, -0.5f, 0.5f);
                    rlTexCoord2f((sideUV.x+sideUV.width)/128.0f, (sideUV.y+sideUV.height)/128.0f);
                    rlVertex3f(0.5f, -0.5f, 0.5f);
                    rlTexCoord2f((sideUV.x+sideUV.width)/128.0f, sideUV.y/128.0f);
                    rlVertex3f(0.5f, 0.5f, 0.5f);
                    rlTexCoord2f(sideUV.x/128.0f, sideUV.y/128.0f);
                    rlVertex3f(-0.5f, 0.5f, 0.5f);

                    // RIGHT face (slightly darker)
                    rlColor4ub(180, 180, 180, 255);
                    rlTexCoord2f(sideUV.x/128.0f, (sideUV.y+sideUV.height)/128.0f);
                    rlVertex3f(0.5f, -0.5f, 0.5f);
                    rlTexCoord2f((sideUV.x+sideUV.width)/128.0f, (sideUV.y+sideUV.height)/128.0f);
                    rlVertex3f(0.5f, -0.5f, -0.5f);
                    rlTexCoord2f((sideUV.x+sideUV.width)/128.0f, sideUV.y/128.0f);
                    rlVertex3f(0.5f, 0.5f, -0.5f);
                    rlTexCoord2f(sideUV.x/128.0f, sideUV.y/128.0f);
                    rlVertex3f(0.5f, 0.5f, 0.5f);
                    rlColor4ub(255, 255, 255, 255);
                rlEnd();
                rlSetTexture(0);
            EndMode3D();
        EndTextureMode();
        
        BlockPreviews.push_back(preview);
    }
}

void DrawHotbar(int selected, vector<int>& hotbarBlocks) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float slotSize = 50.0f;
    float padding = 4.0f;
    float totalWidth = 9 * (slotSize + padding) - padding;
    float startX = sw/2 - totalWidth/2;
    float startY = sh - slotSize - 10;
    float tileSize = 128.0f / 8.0f;
    
    for (int i = 0; i < 9; i++) {
        float x = startX + i * (slotSize + padding);
        Color bg = (i == selected) ? Color{200,200,200,200} : Color{100,100,100,150};
        DrawRectangle(x, startY, slotSize, slotSize, bg);
        DrawRectangleLinesEx({x, startY, slotSize, slotSize}, 2, i == selected ? WHITE : DARKGRAY);
        
        if (hotbarBlocks[i] > 0) {
            int tileID = BlockEntries[hotbarBlocks[i]].TextureSrc[0];
            int tx = tileID % 8;
            int ty = tileID / 8;
            Rectangle source = {tx * tileSize, ty * tileSize, tileSize, tileSize};
            Rectangle dest = {x + 2, startY + 2, slotSize - 4, slotSize - 4};
            //DrawTexturePro(GlobalAtlas, source, dest, {0,0}, 0, WHITE);
            DrawTextureRec(BlockPreviews[hotbarBlocks[i]].texture, 
                   {0, 0, 50, -50}, 
                   {x, startY+2}, WHITE);
        }
    }
}

void DrawInventory(vector<int>& hotbar, int& selected_slot, bool& inventoryOpen) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float slotSize = 50.0f;
    float padding = 4.0f;
    int cols = 9;
    int rows = ((int)BlockEntries.size() - 1) / cols + 1;
    float totalWidth = cols * (slotSize + padding) - padding;
    float totalHeight = rows * (slotSize + padding) - padding;
    float startX = sw/2 - totalWidth/2;
    float startY = sh/2 - totalHeight/2;
    
    bool hovered = false;
    bool afterhovered = false;
    std::string hovername;
    Vector2 mouse ;
    int hoveredid;
    
    // Background
    DrawRectangle(startX - 10, startY - 10, totalWidth + 20, totalHeight + 20, {0,0,0,180});
    DrawRectangleLinesEx({startX - 10, startY - 10, totalWidth + 20, totalHeight + 20}, 2, WHITE);
    
    float tileSize = 128.0f / 8.0f;
    
    // Grid
    for (int i = 1; i < (int)BlockEntries.size(); i++) {
        int col = (i-1) % cols;
        int row = (i-1) / cols;
        float x = startX + col * (slotSize + padding);
        float y = startY + row * (slotSize + padding);
        
        mouse = GetMousePosition();
        hovered = CheckCollisionPointRec(mouse, {x, y, slotSize, slotSize});
        
        DrawRectangle(x, y, slotSize, slotSize, hovered ? Color{160,160,160,200} : Color{100,100,100,150});
        DrawRectangleLinesEx({x, y, slotSize, slotSize}, 2, DARKGRAY);
        
        int tileID = BlockEntries[i].TextureSrc[0];
        int tx = tileID % 8;
        int ty = tileID / 8;
        Rectangle source = {tx * tileSize, ty * tileSize, tileSize, tileSize};
        Rectangle dest = {x + 2, y + 2, slotSize - 4, slotSize - 4};
        DrawTextureRec(BlockPreviews[i].texture, {0, 0, 50, -50}, {x, y+2}, WHITE);
        if(hovered) {
            hovername = BlockEntries[i].name;
            hoveredid = i;
            afterhovered = true;
        }
    }
    if (afterhovered) {
            DrawRectangle(mouse.x + 5, mouse.y-25,MeasureText(hovername.c_str(),16)+10,26,Fade(BLACK,0.7));
            DrawText(hovername.c_str(), mouse.x + 10, mouse.y - 20, 16, WHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                hotbar[selected_slot] = hoveredid;
                inventoryOpen = false;
        }
    }
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "KatchFoundry");
    SetTargetFPS(1000);
    SetExitKey(NULL);
    
    // Block Registration
    NewBlock({}, 0.0f, false, false, false, 0.0f, 0, "Air");
    NewBlock({0,0}, 0.0f, false, false, true, 0.25f, 0, "Grass", SOUND_GRAINY);
    NewBlock({1,0}, 0.0f, false, false, true, 0.55f, 0, "Stone", SOUND_HARD);
    NewBlock({2,0}, 0.0f, false, false, true, 0.20f, 0, "Dirt", SOUND_GRAINY);
    NewBlock({3,0}, 0.0f, false, false, true, 0.55f, 0, "Wood", SOUND_HARD);
    NewBlock({4,0}, 0.0f, true, false, true, 0.35f, 0, "Glass", SOUND_HARD);
    NewBlock({5,0}, 0.15f, true, false, false, 0.0f, 0, "Water");
    NewBlock({6,0}, 0.0f, false, false, true, 0.15f, 0, "Sand", SOUND_SOFT);
    NewBlock({7,0}, 0.0f, false, false, true, 0.45f, 0, "Gravel", SOUND_GRAINY);
    NewBlock({8,9}, 0.0f, false, false, true, 0.45f, 0, "Log", SOUND_HARD);
    NewBlock({10,0}, 0.0f, false, false, true, 0.15f, 0, "Leaf", SOUND_GRAINY);
    NewBlock({11,0}, 0.0f, false, false, true, 0.75f, 0, "Brick", SOUND_HARD);
    NewBlock({12,0}, 0.0f, false, false, true, 0.55f, 0, "Smooth Stone", SOUND_HARD);
    NewBlock({13,0}, 0.0f, false, false, true, 0.1f, 9, "Light", SOUND_HARD);
    NewBlock({14,0}, 0.0f, false, false, true, 0.05f, 0, "Wool", SOUND_SOFT);
    NewBlock({16,15}, 0.0f, false, false, true, 0.2f, 0, "Dynamite", SOUND_GRAINY);
    NewBlock({17,0}, 0.0f, false, false, true, 0.9f, 0, "Iron Block", SOUND_HARD);
    NewBlock({18,0}, 0.0f, false, false, true, 0.8f, 0, "Gold Block", SOUND_HARD);
    NewBlock({19,0}, 0.0f, false, false, true, 1.0f, 0, "Diamond Block", SOUND_HARD);
    NewBlock({20,0}, 0.0f, false, false, false, 0.0f, 5, "Lava");
    BlockEntries[BlockEntries.size()-1].damaging = true;
    BlockEntries[BlockEntries.size()-1].damage = 2;
    
    // Audio + Assets
    InitAudioDevice();
    LoadPack((char*)"Foundry");
    Texture2D Crosshair = LoadTextureFromPack((char*)"UI/crosshair.png");
    Texture2D Cloud = LoadTextureFromPack((char*)"cloud.png");
    
    // Clouds
    Mesh cloudMesh = GenMeshPlane(1024.0f, 1024.0f, 1, 1);
    Model cloudModel = LoadModelFromMesh(cloudMesh);
    cloudModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = Cloud;
    Vector3 CloudPosition  = { 0.0f,    120.0f, 128.0f };
    Vector3 CloudPosition2 = { 1024.0f, 120.0f, 128.0f };
    
    // World + Camera
    vector<Chunk> Chunks;
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 8.0f, 64.0f, 8.0f };
    camera.target   = (Vector3){ 0.0f, 0.0f,  0.0f };
    camera.up       = (Vector3){ 0.0f, 1.0f,  0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Generate Pre-Renders
    GenerateBlockPreviews();
    
    // Player + Hotbar
    Player player = SpawnPlayer((Vector3){ 32.0f, 64.0f, 32.0f });
    vector<int> hotbar = {1,2,3,4,5,6,7,8,9};
    int selected_slot = 0;
    int selected_block = 1;
    
    // Preview Camera
    RenderTexture2D blockPreview = LoadRenderTexture(50, 50);
    Camera3D previewCam = {0};
    previewCam.position = {1.5f, 1.5f, 1.5f};
    previewCam.target = {0.0f, 0.0f, 0.0f};
    previewCam.up = {0.0f, 1.0f, 0.0f};
    previewCam.fovy = 45.0f;
    previewCam.projection = CAMERA_PERSPECTIVE;
    
    // Materials + Shaders
    Material mat = LoadMaterialDefault();
    Material TranslucentChunkMat = LoadMaterialDefault();
    Shader TranslucentShader = LoadShader("Assets/alphacut.vs", "Assets/alphacut.fs");
    
    // Game State
    bool debug        = false;
    bool paused       = false;
    bool song         = true;
    bool inventoryOpen = false;
    bool onTitleScreen = true;
    bool CursorLocked = false;
    bool settingsOpen = false;
    bool survivalMode = false;
    
    // Music
    int currentSong = -1;
    float silenceTimer = 0.0f;
    float nextSongIn = GetRandomValue(35, 140);
    Music Song = Songs[0];
    float musicFadeVolume = 0.0f;
    float musicFadeTarget = 1.0f;
    float musicFadeSpeed = 0.5f;
    
    //Settings
    LoadSettings();
    camera.fovy = Settings.fov;
    SetMusicVolume(Song, Settings.musicVolume);
    
    // Time
    float DayTime   = 0.5f;
    float DayLength = 60.0f * 24.0f;
    Color SkyColor  = SKYBLUE;
    
    // Title Screen
    TitleScreen titleScreen = InitTitleScreen("Foundry");
    std::string worldPath, packName;
    
    while (!WindowShouldClose()) {
        screenWidth  = GetScreenWidth();
        screenHeight = GetScreenHeight();
        SetWindowSize(screenWidth, screenHeight);

        // Title Screen
        if (onTitleScreen) {
            BeginDrawing();
            if (UpdateTitleScreen(titleScreen, worldPath, packName)) {
                SAVE_DIR = worldPath;
                for (char& c : SAVE_DIR) if (c == '\\') c = '/';
                LoadPack((char*)packName.c_str());
                Chunks.clear();
                if (SaveExists()) {
                    InitChunks(Chunks);
                    LoadChunks(Chunks);
                    LoadPlayer(player);
                    auto meta = LoadMeta();
                    DayTime = meta.dayTime;
                    survivalMode = meta.survival;
                    
                } else {
                    GenerateChunks(Chunks);
                    SaveWorld(Chunks, player, DayTime,survivalMode);
                    player.health = 20;
                }
                BuildChunkMeshes(Chunks, DayTime);
                UnloadTitleScreen(titleScreen);
                onTitleScreen = false;
            }
            EndDrawing();
            continue;
        }
        
        // Keybinds
        if (IsKeyPressed(KEY_F3)) debug = !debug;
        if (IsKeyPressed(KEY_M))  song  = !song;
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (inventoryOpen) inventoryOpen = false;
            else paused = !paused;
        }
        if (IsKeyPressed(KEY_E) && !paused) inventoryOpen = !inventoryOpen;
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_S)) SaveWorld(Chunks, player, DayTime,survivalMode);
        
        // Game Update
        if (!paused) {
            if (song) {
                if (currentSong >= 0 && IsMusicStreamPlaying(Songs[currentSong])) {
                UpdateMusicStream(Songs[currentSong]);
                
                float timeLeft = GetMusicTimeLength(Songs[currentSong]) - GetMusicTimePlayed(Songs[currentSong]);
                if (timeLeft < musicFadeSpeed) musicFadeTarget = 0.0f;
                
                musicFadeVolume += (musicFadeTarget - musicFadeVolume) * GetFrameTime() * (1.0f / musicFadeSpeed);
                SetMusicVolume(Songs[currentSong], musicFadeVolume * Settings.musicVolume);
                
            } else {
                silenceTimer += GetFrameTime();
                if (silenceTimer >= nextSongIn) {
                    currentSong = GetRandomValue(0, (int)Songs.size() - 1);
                    musicFadeVolume = 0.0f;
                    musicFadeTarget = 1.0f;
                    PlayMusicStream(Songs[currentSong]);
                    silenceTimer = 0;
                    nextSongIn = GetRandomValue(45, 160);
                    }
                }
            }
            
            player = UpdatePlayer(player, Chunks, Settings.sfxVolume);
            camera.position = {player.Position.x, player.Position.y + 1.6f, player.Position.z};
            if(!inventoryOpen) UpdatePlayerCamera(camera, player, 0.002f);
            DayTime += GetFrameTime() / DayLength;
            if (DayTime >= 1.0f) DayTime = 0.0f;
            SkyColor = GetSkyColor(DayTime);
        }
        
        // Cursor Lock
        bool shouldLockCursor = !paused && !inventoryOpen;
        if (shouldLockCursor) {
            DisableCursor();
            HideCursor();
            SetMousePosition(screenWidth/2, screenHeight/2);
            CursorLocked = true;
        } else {
            if(CursorLocked){
                ShowCursor();
                EnableCursor();
                CursorLocked = false;
            }
        }
        
        // Hotbar Selection
        if (!paused && !inventoryOpen) {
            float scroll = GetMouseWheelMove();
            if (scroll > 0) selected_slot = (selected_slot - 1 + 9) % 9;
            if (scroll < 0) selected_slot = (selected_slot + 1) % 9;
            if (IsKeyPressed(KEY_ONE))   selected_slot = 0;
            if (IsKeyPressed(KEY_TWO))   selected_slot = 1;
            if (IsKeyPressed(KEY_THREE)) selected_slot = 2;
            if (IsKeyPressed(KEY_FOUR))  selected_slot = 3;
            if (IsKeyPressed(KEY_FIVE))  selected_slot = 4;
            if (IsKeyPressed(KEY_SIX))   selected_slot = 5;
            if (IsKeyPressed(KEY_SEVEN)) selected_slot = 6;
            if (IsKeyPressed(KEY_EIGHT)) selected_slot = 7;
            if (IsKeyPressed(KEY_NINE))  selected_slot = 8;
            selected_block = hotbar[selected_slot];
        }
        
        
        // Light
        static int lightUpdateIndex = 0;
        static float lastDayTime = -1.0f;
        float lightStep = 0.0125f;
        
        if (floorf(DayTime / lightStep) != floorf(lastDayTime / lightStep)) {
            lightUpdateIndex = 0;
            lastDayTime = DayTime;
        }
        
        if (lightUpdateIndex < (int)Chunks.size()) {
            Chunk& chunk = Chunks[lightUpdateIndex];
            BuildChunkMesh(chunk, false, Chunks, DayTime);
            BuildChunkMesh(chunk, true, Chunks, DayTime);
            lightUpdateIndex++;
        }
        
        // Render
        BeginDrawing();
        ClearBackground(SkyColor);
        BeginMode3D(camera);
        rlEnableDepthTest();
        
        mat.maps[MATERIAL_MAP_DIFFUSE].texture = GlobalAtlas;
        TranslucentChunkMat.maps[MATERIAL_MAP_DIFFUSE].texture = GlobalAtlas;
        TranslucentChunkMat.shader = TranslucentShader;
        
        // Draw Chunks
        for (int x = 0; x < WORLD_SIZE; x++) {
            for (int z = 0; z < WORLD_SIZE; z++) {
                int index = x + z * WORLD_SIZE;
                Chunk chunk = Chunks[index];
                if (chunk.ChunkMesh.vertexCount > 0)
                    DrawMesh(chunk.ChunkMesh, mat, MatrixTranslate(chunk.ChunkX * CHUNK_WIDTH, 0.0f, chunk.ChunkZ * CHUNK_WIDTH));
                if (chunk.TranslucentChunkMesh.vertexCount > 0)
                    DrawMesh(chunk.TranslucentChunkMesh, TranslucentChunkMat, MatrixTranslate(chunk.ChunkX * CHUNK_WIDTH, 0.0f, chunk.ChunkZ * CHUNK_WIDTH));
            }
        }
        
        // Draw Clouds
        CloudPosition.x  += 2.0f * GetFrameTime();
        CloudPosition2.x += 2.0f * GetFrameTime();
        if (CloudPosition.x  > 1024.0f) CloudPosition.x  -= 2048.0f;
        if (CloudPosition2.x > 1024.0f) CloudPosition2.x -= 2048.0f;
        rlPushMatrix();
            rlTranslatef(CloudPosition.x, CloudPosition.y, CloudPosition.z);
            rlDisableBackfaceCulling();
            DrawModel(cloudModel, Vector3Zero(), 1.0f, WHITE);
            rlEnableBackfaceCulling();
        rlPopMatrix();
        rlPushMatrix();
            rlTranslatef(CloudPosition2.x, CloudPosition2.y, CloudPosition2.z);
            rlDisableBackfaceCulling();
            DrawModel(cloudModel, Vector3Zero(), 1.0f, WHITE);
            rlEnableBackfaceCulling();
        rlPopMatrix();
        
        EndMode3D();
        
        // HUD
        DrawText("KatchFoundry Indev", 5, 10, 20, WHITE);
        DrawText("(Unfinished Build)", 5, 40, 20, YELLOW);
        
        if (debug) {
            DrawRectangle(0,55,250,105,Fade(GRAY, 0.5));
            DrawFPS(5, 60);
            DrawText(TextFormat("XYZ: %.2f, %.2f, %.2f", player.Position.x, player.Position.y, player.Position.z), 5, 80, 20, WHITE);
            DrawText(TextFormat("Local XZ: %.1f, %.1f", fmod(player.Position.x, 16), fmod(player.Position.z, 16)), 5, 100, 20, WHITE);
            DrawText(TextFormat("Chunk XZ: %.0f, %.0f", player.Position.x / 16, player.Position.z / 16), 5, 120, 20, WHITE);
            DrawText(TextFormat("Time: %.4f", DayTime), 5, 140, 20, WHITE);
        }
        
        // Block Interaction + Hotbar
        if (!paused && !inventoryOpen) PlayerBuild(player, Chunks, selected_block,DayTime);
        if(!survivalMode) DrawHotbar(selected_slot, hotbar);
        DrawTextureEx(Crosshair, {(float)screenWidth/2-16, (float)screenHeight/2-16}, 0, 2, WHITE);
        
        // Health
        if (survivalMode) {
            int heartSize = 24;
            int startX = (float)screenWidth/2-(HeartFull.width * 8);
            float startY = screenHeight - 16 - 10 - 70;
            
            for (int i = 0; i < 10; i++) {   // 10 hearts = 20 health
                Rectangle dest = { (float)(startX + i * (heartSize + 2)), (float)startY, (float)heartSize,(float)heartSize};
                
                float heartValue = player.health - (i * 2.0f);
                
                if (heartValue >= 2.0f)
                    DrawTexturePro(HeartFull, {0,0,(float)HeartFull.width,(float)HeartFull.height}, dest, {0,0}, 0, WHITE);
                else if (heartValue >= 1.0f)
                    DrawTexturePro(HeartHalf, {0,0,(float)HeartHalf.width,(float)HeartHalf.height}, dest, {0,0}, 0, WHITE);
                else
                    DrawTexturePro(HeartEmpty, {0,0,(float)HeartEmpty.width,(float)HeartEmpty.height}, dest, {0,0}, 0, WHITE);
            }
        }
        
        // Inventory
        if (inventoryOpen && !survivalMode) DrawInventory(hotbar, selected_slot, inventoryOpen);
        
        // Settings
        if (paused) {
            if (settingsOpen) DrawSettingsMenu(settingsOpen, Song);
            else  {
                int Action = DrawPauseMenu(paused, settingsOpen,Chunks,player,DayTime,survivalMode);
                if(Action == 1) {
                    SaveWorld(Chunks, player, DayTime,survivalMode);
                    Chunks.clear();
                    paused = false;
                    onTitleScreen = true;
                    titleScreen = InitTitleScreen(packName.c_str());
                }
            }
        }
        camera.fovy = Settings.fov;
        SetMusicVolume(Song, Settings.musicVolume);
        
        EndDrawing();
    }
    
    // Cleanup
    CloseWindow();
}