
// ---- Helpers ----

struct WorldEntry {
    std::string name;
    std::string path;
};

struct PackEntry {
    std::string name;
    std::string path;
    Texture2D icon;
};

std::vector<WorldEntry> ScanWorlds() {
    std::vector<WorldEntry> worlds;
    FilePathList files = LoadDirectoryFiles("saves");
    for (int i = 0; i < (int)files.count; i++) {
        if (DirectoryExists(files.paths[i])) {
            WorldEntry e;
            e.path = files.paths[i];
            // Get just the folder name
            e.name = GetFileName(files.paths[i]);
            worlds.push_back(e);
        }
    }
    UnloadDirectoryFiles(files);
    return worlds;
}

std::vector<PackEntry> ScanPacks() {
    std::vector<PackEntry> packs;
    FilePathList files = LoadDirectoryFiles("Assets/Packs");
    for (int i = 0; i < (int)files.count; i++) {
        if (DirectoryExists(files.paths[i])) {
            PackEntry e;
            e.path = files.paths[i];
            e.name = GetFileName(files.paths[i]);
            char iconPath[256];
            snprintf(iconPath, 256, "%s/Icon.png", files.paths[i]);
            e.icon = FileExists(iconPath) ? LoadTexture(iconPath) : Texture2D{0};
            packs.push_back(e);
        }
    }
    UnloadDirectoryFiles(files);
    return packs;
}
bool DrawButton(const char* text, Rectangle rect, Color bg, Color hover, Color textColor) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    DrawRectangleRec(rect, hovered ? hover : bg);
    DrawRectangleLinesEx(rect, 2, BLACK);
    int fontSize = 20;
    int tw = MeasureText(text, fontSize);
    DrawText(text, rect.x + rect.width/2 - tw/2, rect.y + rect.height/2 - fontSize/2, fontSize, textColor);
    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}


enum MenuScene { MENU_MAIN, MENU_WORLDS, MENU_NEWWORLD, MENU_PACKS };

struct TitleScreen {
    MenuScene state = MENU_MAIN;
    Texture2D logo;
    Texture2D background;
    std::string splash;
    std::vector<WorldEntry> worlds;
    std::vector<PackEntry> packs;
    std::string selectedWorld;
    std::string selectedPack;
    float splashWobble = 0.0f;
    float worldListScroll = 0.0f;
};

std::vector<std::string> LoadSplashes(const char* packName) {
    std::vector<std::string> splashes;
    char path[256];
    snprintf(path, 256, "Assets/Packs/%s/splashes.json", packName);
    
    FILE* f = fopen(path, "r");
    if (!f) {
        // FAll back if someone loads it without the splashes 
        splashes = {
            "Insert a splash text here or something!"
        };
        return splashes;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        std::string s = line;
        size_t start = s.find('"');
        if (start == std::string::npos) continue;
        size_t end = s.find('"', start + 1);
        if (end == std::string::npos) continue;
        std::string splash = s.substr(start + 1, end - start - 1);
        if (!splash.empty()) splashes.push_back(splash);
    }
    fclose(f);
    return splashes;
}

TitleScreen InitTitleScreen(const char* packName) {
    TitleScreen ts;
    ts.logo = LoadTextureFromPack((char*)"UI/logo.png");
    ts.background = LoadTextureFromPack((char*)"UI/background.png");
    
    auto splashes = LoadSplashes(packName);
    if (!splashes.empty())
        ts.splash = splashes[GetRandomValue(0, splashes.size() - 1)];
    else
        std::exit(45); // Stop tampering so much!
    
    return ts;
}

void UnloadTitleScreen(TitleScreen& ts) {
    UnloadTexture(ts.logo);
    UnloadTexture(ts.background);
    for (auto& p : ts.packs) if (p.icon.id > 0) UnloadTexture(p.icon);
}

bool UpdateTitleScreen(TitleScreen& ts, std::string& outWorld, std::string& outPack) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    ts.splashWobble += GetFrameTime() * 2.0f;
    
    if (ts.background.id > 0) {
        for (int x = 0; x < sw; x += ts.background.width)
            for (int y = 0; y < sh; y += ts.background.height)
                DrawTexture(ts.background, x, y, GRAY);
    } else {
        ClearBackground(DARKGRAY);
    }

    DrawRectangle(0, 0, sw, sh, {0, 0, 0, 80});

    if (ts.state == MENU_MAIN) {
        if (ts.logo.id > 0) {
            float scale = (float)sw / ts.logo.width * 0.5f;
            float lw = ts.logo.width * scale;
            float lh = ts.logo.height * scale;
            float targetW = 400.0f * ((float)sw / 800.0f);
            float targetH = 256.0f * ((float)sw / 800.0f);
            float logoScale = targetW / ts.logo.width;
            
            DrawTextureEx(ts.logo, {sw/2.0f - targetW/2, sh * 0.1f}, 0, logoScale, WHITE);

        }

        // Splash text
        float wobble = sinf(ts.splashWobble) * 3.0f;
        float splashScale = 1.0f + sinf(ts.splashWobble * 0.5f) * 0.05f;
        int splashSize = (int)(18 * splashScale);
        int sw2 = MeasureText(ts.splash.c_str(), splashSize);
        DrawText(ts.splash.c_str(),
            sw/2 - sw2/2. + (int)wobble,
            (int)((sh+ts.logo.height * ((float)sw / ts.logo.width * 0.3f)) * 0.35f),
            splashSize, YELLOW);

        float bw = 300, bh = 45;
        float bx = sw/2 - bw/2;

        Color stone = {120, 120, 120, 255};
        Color stoneHover = {160, 160, 160, 255};

        if (DrawButton("Play", {bx, sh*0.45f, bw, bh}, stone, stoneHover, WHITE)) {
            ts.worlds = ScanWorlds();
            ts.state = MENU_WORLDS;
        }
        if (DrawButton("Texture Packs", {bx, sh*0.45f + bh + 10, bw, bh}, stone, stoneHover, WHITE)) {
            ts.packs = ScanPacks();
            ts.state = MENU_PACKS;
        }
        if (DrawButton("Quit", {bx, sh*0.45f + (bh+10)*2, bw, bh}, stone, stoneHover, WHITE)) {
            CloseWindow();
            std::exit(0);
            return false;
        }
    } else if (ts.state == MENU_WORLDS) {
        DrawText("Select Save", sw/2 - MeasureText("Select Save", 30)/2, 30, 30, WHITE);

        float bw = 400, bh = 45;
        float bx = sw/2 - bw/2;
        float scroll = GetMouseWheelMove();
        ts.worldListScroll -= scroll * 53.0f;
        float maxScroll = max(0.0f, (float)(ts.worlds.size()) * 53.0f - (sh - 200.0f));
        ts.worldListScroll = Clamp(ts.worldListScroll, 0.0f, maxScroll);
        
        float by = 100 - ts.worldListScroll;

        // make world button
        if (DrawButton("+ New Save", {bx, 100, bw, bh}, {60,120,60,255}, {80,160,80,255}, WHITE)) {
            // rando name
            char name[64];
            snprintf(name, 64, "world%d", GetRandomValue(1000, 9999));
            outWorld = std::string("saves/") + name;
            outPack = ts.selectedPack.empty() ? "Foundry" : ts.selectedPack;
            return true;
        }
        by += bh + 10;
        
        BeginScissorMode(bx, 90, bw, sh - 150);
        for (auto& w : ts.worlds) {
            if (DrawButton(w.name.c_str(), {bx, by, bw, bh}, {80,80,80,255}, {120,120,120,255}, WHITE)) {
                outWorld = w.path;
                outPack = ts.selectedPack.empty() ? "Foundry" : ts.selectedPack;
                EndScissorMode();
                return true;
            }
            by += bh + 8;
        }
        EndScissorMode();

        if (DrawButton("Back", {bx, (float)sh - 60, 150, 40}, {80,80,80,255}, {120,120,120,255}, WHITE))
            ts.state = MENU_MAIN;

    } else if (ts.state == MENU_PACKS) {
        DrawText("Texture Packs", sw/2 - MeasureText("Texture Packs", 30)/2, 30, 30, WHITE);

        float by = 100;
        float bw = 400, bh = 60;
        float bx = sw/2 - bw/2;

        for (auto& p : ts.packs) {
            Rectangle rect = {bx, by, bw, bh};
            Vector2 mouse = GetMousePosition();
            bool hovered = CheckCollisionPointRec(mouse, rect);
            bool selected = ts.selectedPack == p.name;

            Color bg = selected ? Color{60,100,160,255} : Color{80,80,80,255};
            Color hov = selected ? Color{80,120,180,255} : Color{120,120,120,255};
            DrawRectangleRec(rect, hovered ? hov : bg);
            DrawRectangleLinesEx(rect, 2, selected ? GOLD : BLACK);

            if (p.icon.id > 0)
                DrawTextureEx(p.icon, {bx + 5, by + 5}, 0, (bh-10)/(float)p.icon.height, WHITE);

            DrawText(p.name.c_str(), (int)(bx + bh + 10), (int)(by + bh/2 - 10), 20, WHITE);

            if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                ts.selectedPack = p.name;                

            by += bh + 8;
        }

        if (DrawButton("Back", {bx, (float)sh - 60, 150, 40}, {80,80,80,255}, {120,120,120,255}, WHITE)) {
            LoadPack((char*)ts.selectedPack.c_str());
            ts.state = MENU_MAIN;
            ts.logo = LoadTextureFromPack((char*)"UI/logo.png");
            ts.background = LoadTextureFromPack((char*)"UI/background.png");
            auto splashes = LoadSplashes(ts.selectedPack.c_str());
            if (!splashes.empty())
                ts.splash = splashes[GetRandomValue(0, splashes.size() - 1)];
            else
                std::exit(45); // Bruvaroo
        }
    }

    return false;
}