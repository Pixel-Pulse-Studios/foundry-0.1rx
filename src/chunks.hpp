#pragma once
#include "blocks.hpp"
#include "noise.hpp"

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 128

#ifdef PLATFORM_WEB
    #define WORLD_SIZE 8
#else
    #define WORLD_SIZE 16
#endif

typedef struct {
    unsigned char lights;
} BlockProperties;

typedef struct {
    vector<int> blocks;
    vector<BlockProperties> Properties;
    Mesh ChunkMesh;
    Mesh TranslucentChunkMesh;
    bool ChunkUpdated;
    int ChunkX;
    int ChunkZ;
} Chunk;

typedef struct {
    bool hit;
    Vector3 point;
    Vector3 normal;
    int blockX, blockY, blockZ;
    int face;
    float distance;
} RaycastHit;

bool IsFaceCovered(const Chunk& chunk, int x, int y, int z, int face, const vector<Chunk>& chunks) {
    int otherX = x;
    int otherY = y;
    int otherZ = z;

    switch (face) {
        case 1: otherZ++; break;
        case 2: otherZ--; break;
        case 3: otherX++; break;
        case 4: otherX--; break;
        case 5: otherY++; break;
        case 6: otherY--; break;
        default: return false;
    }

    if (otherY < 0 || otherY >= CHUNK_HEIGHT) return false;

    int neighborCX = chunk.ChunkX;
    int neighborCZ = chunk.ChunkZ;

    if (otherX < 0) {
        neighborCX -= 1;
        otherX = CHUNK_WIDTH - 1;
    } else if (otherX >= CHUNK_WIDTH) {
        neighborCX += 1;
        otherX = 0;
    }

    if (otherZ < 0) {
        neighborCZ -= 1;
        otherZ = CHUNK_WIDTH - 1;
    } else if (otherZ >= CHUNK_WIDTH) {
        neighborCZ += 1;
        otherZ = 0;
    }

    const Chunk* targetChunk = &chunk;
    if (neighborCX != chunk.ChunkX || neighborCZ != chunk.ChunkZ) {
        targetChunk = nullptr;
        for (int i = 0; i < (int)chunks.size(); i++) {
            if (chunks[i].ChunkX == neighborCX && chunks[i].ChunkZ == neighborCZ) {
                targetChunk = &chunks[i];
                break;
            }
        }
        if (targetChunk == nullptr) return false;
    }

    int Index = x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH;
    int otherIndex = otherX + otherZ * CHUNK_WIDTH + otherY * CHUNK_WIDTH * CHUNK_WIDTH;

    int blockId2 = chunk.blocks[Index];
    int blockId = targetChunk->blocks[otherIndex];

    if (blockId <= 0 || blockId >= (int)BlockEntries.size()) return false;

    BlockEntry& entry = BlockEntries[blockId];

    if (entry.translucent || entry.transparency > 0) {
        if (blockId == blockId2) return true;
        return false;
    }

    return true;
}

bool IsBlockCovered(const Chunk& chunk, int x, int y, int z, const vector<Chunk>& chunks) {
    for (int face = 1; face <= 6; face++) {
        if (!IsFaceCovered(chunk, x, y, z, face, chunks)) {
            return false;
        }
    }
    return true;
}

int GetBlockIndex(int x, int y, int z) {
    return x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH;
}

bool IsValidBlockCoord(int x, int y, int z) {
    return x >= 0 && x < CHUNK_WIDTH &&
           y >= 0 && y < CHUNK_HEIGHT &&
           z >= 0 && z < CHUNK_WIDTH;
}

bool IsBlockSolid(const Chunk& chunk, int x, int y, int z) {
    if (!IsValidBlockCoord(x, y, z)) return false;
    return chunk.blocks[GetBlockIndex(x, y, z)] != 0;
}

char GetDefaultLightLevel(int y) { 
    if(y > 20) return 9; 
    if(y < 11) return 0; 
    return y - 11; 
}

struct Node {
    int x, y, z;
};

std::vector<BlockProperties> UpdateLight(Chunk& chunk, vector<Chunk>& chunks, float DayTime = 0.5f) {
    int size = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH;
    chunk.Properties.assign(size, {});

    std::queue<Node> q;

    auto indexOf = [](int x, int y, int z) {
        return x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH;
    };

    unsigned char skyLight;
    
    if (DayTime < 0.25f) {
        float t = DayTime / 0.25f;
        skyLight = (unsigned char)(1 + t * 8); // 1 -> 9
    } else if (DayTime < 0.5f) {
        skyLight = 9;
    } else if (DayTime < 0.75f) {
        skyLight = 9;
    } else {
        float t = (DayTime - 0.75f) / 0.25f;
        skyLight = (unsigned char)(9 - t * 8); // 9 -> 1
    }
    
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_WIDTH; z++) {
                int index = indexOf(x, y, z);
                int blockId = chunk.blocks[index];
                unsigned char light = BlockEntries[blockId].lightLevel;
                if (light > 0) {
                    chunk.Properties[index].lights = light;
                    q.push({x, y, z});
                }
            }
        }
    }

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int z = 0; z < CHUNK_WIDTH; z++) {
            for (int y = CHUNK_HEIGHT - 1; y >= 0; y--) {
                int index = indexOf(x, y, z);
                int blockId = chunk.blocks[index];
                bool solid = blockId != 0 
                          && !BlockEntries[blockId].translucent 
                          && BlockEntries[blockId].transparency == 0;

                if (solid) break;

                if (skyLight > chunk.Properties[index].lights) {
                    chunk.Properties[index].lights = skyLight;
                    q.push({x, y, z});
                }
            }
        }
    }

    int dx[6] = {1, -1, 0, 0, 0, 0};
    int dy[6] = {0, 0, 1, -1, 0, 0};
    int dz[6] = {0, 0, 0, 0, 1, -1};

    while (!q.empty()) {
        Node n = q.front();
        q.pop();

        int currentIndex = indexOf(n.x, n.y, n.z);
        int currentLight = chunk.Properties[currentIndex].lights;

        if (currentLight <= 1) continue;
        
        int blockId = chunk.blocks[currentIndex];
        bool isSolid = blockId != 0 
                    && !BlockEntries[blockId].translucent 
                    && BlockEntries[blockId].transparency == 0;
        bool isLightSource = BlockEntries[blockId].lightLevel > 0;
        if (isSolid && !isLightSource) continue;

        for (int i = 0; i < 6; i++) {
            int nx = n.x + dx[i];
            int ny = n.y + dy[i];
            int nz = n.z + dz[i];
        
            if (nx < 0 || nx >= CHUNK_WIDTH ||
            ny < 0 || ny >= CHUNK_HEIGHT ||
            nz < 0 || nz >= CHUNK_WIDTH)
            continue;
            
            int neighborIndex = indexOf(nx, ny, nz);
            unsigned char newLight = currentLight - 1;
        
            if (chunk.Properties[neighborIndex].lights < newLight) {
                chunk.Properties[neighborIndex].lights = newLight;
                q.push({nx, ny, nz});
            }
        }
    }

    return chunk.Properties;
}

Chunk InitChunk(int cx, int cz) {
    Chunk chunk;
    chunk.ChunkMesh = {0};
    chunk.TranslucentChunkMesh = {0};
    chunk.ChunkUpdated = false;
    
    chunk.blocks.resize(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH);
    chunk.Properties.resize(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH);
    
    chunk.ChunkX = cx;
    chunk.ChunkZ = cz;
    
    return chunk;
}


float GetHeight(float wx, float wz, float seed = 1234) {
    float h = 32.0f;
    h += stb_perlin_noise3((wx + seed) * 0.008f, 0.0f,   (wz + seed) * 0.008f, 0, 0, 0) * 20.0f;
    h += stb_perlin_noise3((wx + seed) * 0.04f,  0.0f,   (wz + seed) * 0.04f,  0, 0, 0) * 6.0f;
    h += stb_perlin_noise3((wx + seed) * 0.1f,   0.0f,   (wz + seed) * 0.1f,   0, 0, 0) * 2.0f;
    return h;
}

float GetTemperature(float wx, float wz, float seed) {
    float t = stb_perlin_noise3((wx + seed) * 0.004f, 100.0f, (wz + seed) * 0.004f, 0, 0, 0) * 0.6f
            + stb_perlin_noise3((wx + seed) * 0.01f,  100.0f, (wz + seed) * 0.01f,  0, 0, 0) * 0.3f
            + stb_perlin_noise3((wx + seed) * 0.03f,  100.0f, (wz + seed) * 0.03f,  0, 0, 0) * 0.1f;
    
    return (t + 1.0f) * 0.5f;
}

float GetTreeNoise(float wx, float wz, float seed) {
    return stb_perlin_noise3((wx + seed) * 0.1f, 200.0f, (wz + seed) * 0.1f, 0, 0, 0);
}

void PlaceTree(Chunk& chunk, int x, int groundY, int z) {
    int trunkHeight = 4 + (rand() % 3);
    
    for (int y = groundY; y < groundY + trunkHeight; y++) {
        if (!IsValidBlockCoord(x, y, z)) continue;
        chunk.blocks[GetBlockIndex(x, y, z)] = 9;
    }
    
    int leafY = groundY + trunkHeight;
    for (int ly = leafY - 1; ly <= leafY + 1; ly++) {
        for (int lx = x - 2; lx <= x + 2; lx++) {
            for (int lz = z - 2; lz <= z + 2; lz++) {
                if (!IsValidBlockCoord(lx, ly, lz)) continue;
                int idx = GetBlockIndex(lx, ly, lz);
                if (chunk.blocks[idx] == 0)
                    chunk.blocks[idx] = 10;
            }
        }
    }
}

#define DESERT_TEMP 0.55f

Chunk GenerateChunk(int cx, int cz, vector<Chunk>& chunks, int seed = 1234) {
    Chunk chunk = InitChunk(cx,cz);
    
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_WIDTH; z++) {

                int index = x +
                            z * CHUNK_WIDTH +
                            y * CHUNK_WIDTH * CHUNK_WIDTH;
                
                float fx = (float)(x + cx * CHUNK_WIDTH);
                float fz = (float)(z + cz * CHUNK_WIDTH);
                
                float ground_height_f = GetHeight(fx,fz,seed);
                float temp = GetTemperature(fx, fz, seed);
                
                int ground_height = (int)ground_height_f;
                if (temp > DESERT_TEMP) {
                    if (y <= ground_height && y >= ground_height-2)  chunk.blocks[index] = 7; 
                    else if (y < ground_height)  chunk.blocks[index] = 2;
                } else {
                    if (y == ground_height)  chunk.blocks[index] = 1; 
                    else if (y < ground_height-4)  chunk.blocks[index] = 2; 
                    else if (y < ground_height)  chunk.blocks[index] = 3;
                }
            }
        }
    }
    
    for (int x = 2; x < CHUNK_WIDTH - 2; x++) {
        for (int z = 2; z < CHUNK_WIDTH - 2; z++) {
            float fx = (float)(x + cx * CHUNK_WIDTH);
            float fz = (float)(z + cz * CHUNK_WIDTH);
            float temp = GetTemperature(fx, fz, seed);
            
            float treeNoise = GetTreeNoise(fx,fz,seed);
            if(treeNoise > 0.6f && temp < DESERT_TEMP) {
                int groundY = (int)GetHeight(fx, fz, seed);
                PlaceTree(chunk, x, groundY + 1, z);
            }
        }
    }
    
    UpdateLight(chunk, chunks);
    return chunk;
}

void InitChunks(vector<Chunk>& chunks) {
    for(int x = 0; x < WORLD_SIZE; x++) {
            for(int z = 0; z < WORLD_SIZE; z++) {
                int index = x + z * WORLD_SIZE;
                chunks.push_back(InitChunk(x, z));
        }
    }
}

int GenerateChunks(vector<Chunk>& chunks, int seed = -1) {
    if(seed == -1) seed = GetRandomValue(0, 999999);
    for(int x = 0; x < WORLD_SIZE; x++) {
            for(int z = 0; z < WORLD_SIZE; z++) {
                int index = x + z * WORLD_SIZE;
                chunks.push_back(GenerateChunk(x, z, chunks, seed));
        }
    }
    return seed;
}