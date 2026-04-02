
#define SAVE_VERSION 3
std::string SAVE_DIR = "saves/World_";

void CreateSaveDir() {
#ifdef _WIN32
    mkdir("saves");
    mkdir(SAVE_DIR.c_str());
#else
    mkdir("saves", 0777);
    mkdir(SAVE_DIR.c_str(), 0777);
#endif
}


void SaveChunks(const vector<Chunk>& chunks) {
    CreateSaveDir();
    FILE* f = fopen((SAVE_DIR + "/chunks.sav").c_str(), "wb");
    if (!f) return;

    int version = SAVE_VERSION;
    fwrite(&version, sizeof(int), 1, f);

    int count = chunks.size();
    fwrite(&count, sizeof(int), 1, f);

    for (const Chunk& chunk : chunks) {
        fwrite(&chunk.ChunkX, sizeof(int), 1, f);
        fwrite(&chunk.ChunkZ, sizeof(int), 1, f);

        int i = 0;
        int total = chunk.blocks.size();
        while (i < total) {
            int current = chunk.blocks[i];
            int runLength = 1;
            while (i + runLength < total &&
                   chunk.blocks[i + runLength] == current &&
                   runLength < 32767) {
                runLength++;
            }
            fwrite(&current, sizeof(int), 1, f);
            fwrite(&runLength, sizeof(int), 1, f);
            i += runLength;
        }

        int sentinel = -1;
        fwrite(&sentinel, sizeof(int), 1, f);
        fwrite(&sentinel, sizeof(int), 1, f);
    }

    fclose(f);
}

bool LoadChunks(vector<Chunk>& chunks) {
    FILE* f = fopen((SAVE_DIR + "/chunks.sav").c_str(), "rb");
    if (!f) { TraceLog(LOG_WARNING, "chunks.sav not found!"); return false; }

    int version;
    fread(&version, sizeof(int), 1, f);
    TraceLog(LOG_INFO, "Version: %d expected %d", version, SAVE_VERSION);
    if (version < 3 || version > SAVE_VERSION) { fclose(f); return false; }
    
    int count;
    fread(&count, sizeof(int), 1, f);
    TraceLog(LOG_INFO, "Chunk count in save: %d, chunks vector size: %d", count, (int)chunks.size());
    if (count <= 0 || count > 1024) { 
        TraceLog(LOG_WARNING, "Invalid chunk count: %d", count);
        fclose(f); 
        return false; 
    }

    for (int i = 0; i < count; i++) {
        int cx, cz;
        fread(&cx, sizeof(int), 1, f);
        fread(&cz, sizeof(int), 1, f);

        vector<int> tempBlocks;
        tempBlocks.reserve(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH);

        while (true) {
            int blockId, runLength;
            fread(&blockId, sizeof(int), 1, f);
            fread(&runLength, sizeof(int), 1, f);
            if (blockId == -1 && runLength == -1) break;
            for (int r = 0; r < runLength; r++)
                tempBlocks.push_back(blockId);
        }

        for (Chunk& chunk : chunks) {
            if (chunk.ChunkX == cx && chunk.ChunkZ == cz) {
                chunk.blocks = tempBlocks;
                chunk.ChunkUpdated = true;
                break;
            }
        }
    }

    fclose(f);
    return true;
}

void SavePlayer(const Player& player) {
    CreateSaveDir();
    FILE* f = fopen((SAVE_DIR + "/player.sav").c_str(), "wb");
    if (!f) return;

    fwrite(&player.Position.x, sizeof(float), 1, f);
    fwrite(&player.Position.y, sizeof(float), 1, f);
    fwrite(&player.Position.z, sizeof(float), 1, f);
    fwrite(&player.Yaw,        sizeof(float), 1, f);
    fwrite(&player.Pitch,      sizeof(float), 1, f);

    fclose(f);
}

bool LoadPlayer(Player& player) {
    FILE* f = fopen((SAVE_DIR + "/player.sav").c_str(), "rb");
    if (!f) return false;

    fread(&player.Position.x, sizeof(float), 1, f);
    fread(&player.Position.y, sizeof(float), 1, f);
    fread(&player.Position.z, sizeof(float), 1, f);
    fread(&player.Yaw,        sizeof(float), 1, f);
    fread(&player.Pitch,      sizeof(float), 1, f);

    fclose(f);
    return true;
}

void SaveMeta(float dayTime,int survival) {
    CreateSaveDir();
    FILE* f = fopen((SAVE_DIR + "/world.json").c_str(), "w");
    if (!f) return;

    fprintf(f, "{\n");
    fprintf(f, "    \"version\": %d,\n", SAVE_VERSION);
    fprintf(f, "    \"daytime\": %.4f,\n", dayTime);
    fprintf(f, "    \"survival\": %i\n", survival);
    fprintf(f, "}\n");

    fclose(f);
}

struct WorldMeta {
    float dayTime;
    bool survival;
};

WorldMeta LoadMeta() {
    FILE* f = fopen((SAVE_DIR + "/world.json").c_str(), "r");
    if (!f) return { 0.5f, false };
    
    float dayTime = 0.5f;
    int survival = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        sscanf(line, " \"daytime\": %f", &dayTime);
        sscanf(line, " \"survival\": %d", &survival);
    }
        TraceLog(LOG_INFO, "Loaded meta — dayTime: %.4f, survival: %d", dayTime, survival);

    fclose(f);
    return { dayTime, (bool)survival };
}

void SaveWorld(const vector<Chunk>& chunks, const Player& player, float dayTime, int survival) {
    SaveChunks(chunks);
    SavePlayer(player);
    SaveMeta(dayTime,survival);
}

bool SaveExists() {
    FILE* f = fopen((SAVE_DIR + "/chunks.sav").c_str(), "rb");
    if (!f) return false;
    fclose(f);
    return true;
}