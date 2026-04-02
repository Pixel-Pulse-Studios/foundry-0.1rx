
Texture2D EmptyTexture[6] = {};

vector<RenderTexture2D> BlockPreviews;
Texture2D HeartFull, HeartHalf, HeartEmpty;

vector<Music> Songs;
Sound SFX_Hurt;

#define PATH_BUFFER 256



Texture2D LoadAtlas(char *name) {
    char path[PATH_BUFFER];
    snprintf(path, PATH_BUFFER, "Assets/Packs/%s/terrain.png", name);
    return LoadTexture(path);
}

string packname;

Sound LoadSFXFromPack(char *name) {
    char path[PATH_BUFFER];
    snprintf(path, PATH_BUFFER, "Assets/Packs/%s/Audio/%s", packname.c_str(), name);
    return LoadSound(path);
}

Music LoadSongFromPack(char *name) {
    char path[PATH_BUFFER];
    snprintf(path, PATH_BUFFER, "Assets/Packs/%s/Audio/Music/%s", packname.c_str(), name);
    return LoadMusicStream(path);
}

vector<Music> LoadMusicFromPack(char* packName) {
    vector<Music> songs;
    int i = 1;
    while (true) {
        char path[PATH_BUFFER];
        snprintf(path, PATH_BUFFER, "Assets/Packs/%s/Audio/Music/%d.mp3", packName, i);
        if (!FileExists(path)) break;
        Music song = LoadMusicStream(path);
        songs.push_back(song);
        i++;
    }
    return songs;
}

Texture2D LoadPackIcon(char *name) {
    char path[PATH_BUFFER];
    snprintf(path, PATH_BUFFER, "Assets/Packs/%s/Icon.png", name);
    return LoadTexture(path);
}

Texture2D LoadTextureFromPack(char *name) {
    char path[PATH_BUFFER];
    snprintf(path, PATH_BUFFER, "Assets/Packs/%s/%s", packname.c_str(), name);
    return LoadTexture(path);
}

void LoadPack(char *name) {
    char path[PATH_BUFFER];
    packname = name;
    
    Texture2D Atlas = LoadAtlas(name);
    GlobalAtlas = Atlas;
    
    Songs = LoadMusicFromPack(name);
    for (Music& s : Songs) s.looping = false; // Disable looping because ew
    
    SFX_BreakHard   = LoadSFXFromPack((char*)"/breakhard.wav");
    SFX_BreakGrainy = LoadSFXFromPack((char*)"/breakgrainy.wav");
    SFX_BreakSoft   = LoadSFXFromPack((char*)"/breaksoft.wav");
    SFX_PlaceHard   = LoadSFXFromPack((char*)"/placehard.wav");
    SFX_PlaceGrainy   = LoadSFXFromPack((char*)"/placegrainy.wav");
    SFX_PlaceSoft   = LoadSFXFromPack((char*)"/placesoft.wav");
    
    SFX_StepHard   = LoadSFXFromPack((char*)"/stephard.wav");
    SFX_StepGrainy = LoadSFXFromPack((char*)"/stepgrainy.wav");
    SFX_StepGrainy2 = LoadSFXFromPack((char*)"/stepgrainy2.wav");
    SFX_StepSoft = LoadSFXFromPack((char*)"/stepsoft.wav");
    SFX_StepSoft2 = LoadSFXFromPack((char*)"/stepsoft2.wav");
    
    SFX_Hurt = LoadSFXFromPack((char*)"/hurt.wav");
    
    HeartFull  = LoadTextureFromPack((char*)"UI/heart.png");
    HeartHalf  = LoadTextureFromPack((char*)"UI/halfheart.png");
    HeartEmpty = LoadTextureFromPack((char*)"UI/drainedheart.png");
}

