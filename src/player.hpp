#pragma once
#include "raylib.h"
#pragma once
#include "chunks.hpp"

typedef struct {
    Vector3 Position;
    Vector3 Size;
    Vector3 Velocity;
    bool Grounded;
    float Yaw;
    float Pitch;
    float airtime;
    float stepTimer = 0.0f;
    bool variant;
    float health;
    float fallVelocity;
}Player;

Vector3 DefaultPlayerSize = (Vector3){1,1.5,1};

Player SpawnPlayer(Vector3 Pos) {
    Player NewPlayer = {};
    NewPlayer.Position = Pos;
    NewPlayer.Size = DefaultPlayerSize;
    NewPlayer.Velocity = (Vector3){0,0,0};
    NewPlayer.Grounded = false;
    NewPlayer.Yaw = 0.0f;
    NewPlayer.Pitch = 0.0f;
    NewPlayer.stepTimer = 0.0f;
    NewPlayer.variant = false;
    NewPlayer.health = 17.f;
    NewPlayer.fallVelocity = 0.0f;
    return NewPlayer;
}

Vector2 GetChunk(Player player) {
    return (Vector2){floor(player.Position.x / 16), floor(player.Position.z / 16)};
}

Vector2 GetLocalXZ(Player player) {
    return (Vector2){(float)(((int)floor(player.Position.x) % CHUNK_WIDTH + CHUNK_WIDTH) % CHUNK_WIDTH), (float)(((int)floor(player.Position.z) % CHUNK_WIDTH + CHUNK_WIDTH) % CHUNK_WIDTH)};
}

int BlockAtPos(Vector3 Pos,vector<Chunk>& chunks) {
    Player temp = {};
    temp.Position = Pos;
    Vector2 GChunk = GetChunk(temp);
    Vector2 Local = GetLocalXZ(temp);
    
    int ChunkIndex = GChunk.y + GChunk.x * WORLD_SIZE;
    int LocalIndex = Local.x + Local.y * CHUNK_WIDTH + (int)floor(Pos.y) * CHUNK_WIDTH * CHUNK_WIDTH;
    
    if(ChunkIndex < 0 || ChunkIndex >= (int)chunks.size()) return 0;
    
    if(Local.x < 0 || Local.x >= CHUNK_WIDTH) return 0;
    if(Local.y < 0 || Local.y >= CHUNK_WIDTH) return 0;
    if(Pos.y < 0 || Pos.y >= CHUNK_HEIGHT) return 0;
    
    Chunk chunk = chunks[ChunkIndex];
    
    return chunk.blocks[LocalIndex];
}

void SetBlockAtPos(Vector3 Pos,vector<Chunk>& chunks,int id, float DayTime = 0.0f) {
    Player temp = {};
    temp.Position = Pos;
    Vector2 GChunk = GetChunk(temp);
    Vector2 Local = GetLocalXZ(temp);
    
    int ChunkIndex = GChunk.y + GChunk.x * WORLD_SIZE;
    int LocalIndex = Local.x + Local.y * CHUNK_WIDTH + (int)floor(Pos.y) * CHUNK_WIDTH * CHUNK_WIDTH;
    
    if(ChunkIndex < 0 || ChunkIndex >= (int)chunks.size()) return;
    
    if(Local.x < 0 || Local.x >= CHUNK_WIDTH) return;
    if(Local.y < 0 || Local.y >= CHUNK_WIDTH) return;
    if(Pos.y < 0 || Pos.y >= CHUNK_HEIGHT) return;
    
    Chunk& chunk = chunks[ChunkIndex];
    
    chunk.blocks[LocalIndex] = id;
    chunk.Properties[LocalIndex].lights = BlockEntries[id].lightLevel;
    
    BuildChunkMesh(chunk,false,chunks,DayTime);
    BuildChunkMesh(chunk,true,chunks,DayTime);
    if(Local.y == 0) {
        if(GChunk.y != 0) { 
            int chunkindex = (GChunk.y-1) + GChunk.x * WORLD_SIZE;
            BuildChunkMesh(chunks[chunkindex],false,chunks, DayTime);
            BuildChunkMesh(chunks[chunkindex],true,chunks, DayTime);
        }
    }
    if(Local.y == CHUNK_WIDTH-1) {
        if(GChunk.y < WORLD_SIZE) { 
            int chunkindex = (GChunk.y+1) + GChunk.x * WORLD_SIZE;
            BuildChunkMesh(chunks[chunkindex],false,chunks,DayTime);
            BuildChunkMesh(chunks[chunkindex],true,chunks,DayTime);
        }
    }
    if(Local.x == 0) {
        if(GChunk.x != 0) { 
            int chunkindex = GChunk.y + (GChunk.x-1) * WORLD_SIZE;
            BuildChunkMesh(chunks[chunkindex],false,chunks,DayTime);
            BuildChunkMesh(chunks[chunkindex],true,chunks,DayTime);
        }
    }
    if(Local.x == CHUNK_WIDTH-1) {
        if(GChunk.x < WORLD_SIZE) { 
            int chunkindex = GChunk.y + (GChunk.x+1) * WORLD_SIZE;
            BuildChunkMesh(chunks[chunkindex],false,chunks);
            BuildChunkMesh(chunks[chunkindex],true,chunks);
        }
    }
}

void UpdatePlayerCamera(Camera3D& camera, Player& player, float sensitivity) {
    Vector2 mouseDelta = GetMouseDelta();

    player.Yaw   -= mouseDelta.x * sensitivity;
    player.Pitch -= mouseDelta.y * sensitivity;

    if (player.Pitch >  89.0f * DEG2RAD) player.Pitch =  89.0f * DEG2RAD;
    if (player.Pitch < -89.0f * DEG2RAD) player.Pitch = -89.0f * DEG2RAD;

    Vector3 forward = {
        cosf(player.Pitch) * sinf(player.Yaw),
        sinf(player.Pitch),
        cosf(player.Pitch) * cosf(player.Yaw)
    };

    camera.position = {
        player.Position.x,
        player.Position.y + player.Size.y - 0.2f,
        player.Position.z
    };

    camera.target = Vector3Add(camera.position, forward);
}

bool IsSolidAt(Vector3 pos, vector<Chunk>& chunks) {
    int b = BlockAtPos(pos, chunks);
    return b > 0 && b < (int)BlockEntries.size() && BlockEntries[b].solid;
}

bool HasGroundBelow(Vector3 pos, vector<Chunk>& chunks, float halfX = 0.4f) {
    float checkY = pos.y - 0.1f;
    return IsSolidAt({pos.x + halfX, checkY, pos.z}, chunks) ||
           IsSolidAt({pos.x - halfX, checkY, pos.z}, chunks) ||
           IsSolidAt({pos.x, checkY, pos.z + halfX}, chunks) ||
           IsSolidAt({pos.x, checkY, pos.z - halfX}, chunks) ||
           IsSolidAt({pos.x, checkY, pos.z}, chunks);
}

void PlayerCollision(Player& player, vector<Chunk>& chunks) {
    float halfX = player.Size.x * 0.5f;
    float halfZ = player.Size.x * 0.5f;

    float yLevels[] = {
        player.Position.y + 0.1f,
        player.Position.y + player.Size.y * 0.5f,
        player.Position.y + player.Size.y - 0.1f
    };

    for (float checkY : yLevels) {
        if (BlockAtPos({player.Position.x + halfX, checkY, player.Position.z}, chunks)) {
            int b = BlockAtPos({player.Position.x + halfX, checkY, player.Position.z}, chunks);
            if (b > 0 && BlockEntries[b].solid)
                player.Position.x = floorf(player.Position.x + halfX) - halfX;
        }
        if (BlockAtPos({player.Position.x - halfX, checkY, player.Position.z}, chunks)) {
            int b = BlockAtPos({player.Position.x - halfX, checkY, player.Position.z}, chunks);
            if (b > 0 && BlockEntries[b].solid)
                player.Position.x = ceilf(player.Position.x - halfX) + halfX;
        }
        if (BlockAtPos({player.Position.x, checkY, player.Position.z + halfZ}, chunks)) {
            int b = BlockAtPos({player.Position.x, checkY, player.Position.z + halfZ}, chunks);
            if (b > 0 && BlockEntries[b].solid)
                player.Position.z = floorf(player.Position.z + halfZ) - halfZ;
        }
        if (BlockAtPos({player.Position.x, checkY, player.Position.z - halfZ}, chunks)) {
            int b = BlockAtPos({player.Position.x, checkY, player.Position.z - halfZ}, chunks);
            if (b > 0 && BlockEntries[b].solid)
                player.Position.z = ceilf(player.Position.z - halfZ) + halfZ;
        }
        if (BlockAtPos({player.Position.x, checkY-0.1f, player.Position.z}, chunks)) {
            int b = BlockAtPos({player.Position.x, player.Position.y - 0.1f, player.Position.z}, chunks);
            if (b > 0 && BlockEntries[b].solid)
                player.Position.y = ceilf(player.Position.y - 0.1f) + 0.1f;
        }
    }
}


// Net
struct PlayerPacket {
    float x, y, z;
    float yaw, pitch;
};

Player UpdatePlayer(Player player, vector<Chunk>& chunks, float sfxVolume = 1.0f, bool multiplayer = false) {
    int lasthealth = player.health;
    
    float dt = GetFrameTime();

    int BlockBelowPlayer = BlockAtPos(
        (Vector3){player.Position.x, player.Position.y - player.Size.y + 1, player.Position.z},
        chunks
    );

    bool hasGround =
        (BlockAtPos({player.Position.x + 0.4f, player.Position.y - 0.1f, player.Position.z}, chunks) > 0) ||
        (BlockAtPos({player.Position.x - 0.4f, player.Position.y - 0.1f, player.Position.z}, chunks) > 0) ||
        (BlockAtPos({player.Position.x, player.Position.y - 0.1f, player.Position.z + 0.4f}, chunks) > 0) ||
        (BlockAtPos({player.Position.x, player.Position.y - 0.1f, player.Position.z - 0.4f}, chunks) > 0) ||
        (BlockAtPos({player.Position.x, player.Position.y - 0.1f, player.Position.z}, chunks) > 0);
    
    if (BlockBelowPlayer > 0 && BlockBelowPlayer < (int)BlockEntries.size() && BlockEntries[BlockBelowPlayer].solid)
    {
        player.Velocity.y = 0;
        player.Grounded = true;
    }
    else
    {
        player.Velocity.y -= 34.5f * dt;
        if (player.Velocity.y < -85.5f) player.Velocity.y = -85.5f;
        player.Grounded = false;
    }

    if (player.Grounded && IsKeyDown(KEY_SPACE))
        player.Velocity.y = 7.35f;
    
    if (!player.Grounded)
        player.fallVelocity = player.Velocity.y;
    
    if (player.Grounded && player.fallVelocity < -15.0f) {
        float damage = (-player.fallVelocity - 15.0f) * 0.5f;
        player.health -= (int)damage;
        player.fallVelocity = 0.0f;
    }
    
    int blockAtFeet = BlockAtPos(player.Position, chunks);
    if (blockAtFeet > 0 && blockAtFeet < (int)BlockEntries.size()) {
        if (BlockEntries[blockAtFeet].damaging) {
        player.health -= BlockEntries[blockAtFeet].damage * dt;
        }
    }
    
    if (BlockBelowPlayer > 0 && BlockBelowPlayer < (int)BlockEntries.size()) {
        if (BlockEntries[BlockBelowPlayer].damaging) {
        player.health -= BlockEntries[BlockBelowPlayer].damage * dt;
        }
    }

    Vector3 forward = {
        sinf(player.Yaw),
        0,
        cosf(player.Yaw)
    };

    Vector3 right = {
        cosf(player.Yaw),
        0,
        -sinf(player.Yaw)
    };

    float speed = 0.12f;
    if (IsKeyDown(KEY_LEFT_SHIFT)) speed = 0.06f;
    if (IsKeyDown(KEY_LEFT_CONTROL)) speed = 0.16f;

    Vector3 movement = {0, 0, 0};

    if (IsKeyDown(KEY_W)) movement = Vector3Add(movement, forward);
    if (IsKeyDown(KEY_S)) movement = Vector3Add(movement, Vector3Scale(forward, -1.0f));
    if (IsKeyDown(KEY_D)) movement = Vector3Add(movement, Vector3Scale(right, -1.0f));
    if (IsKeyDown(KEY_A)) movement = Vector3Add(movement, right);

    if (IsKeyDown(KEY_LEFT_SHIFT))
    {
        Vector3 futurePos = Vector3Add(player.Position, movement);
        int blockBelow = BlockAtPos(
            (Vector3){futurePos.x, player.Position.y - player.Size.y + 1, futurePos.z},
            chunks
        );

        if (blockBelow == 0 && player.Grounded)
        {
            movement = {0, 0, 0};
        }
    }

    if (Vector3Length(movement) > 0)
    {
        movement = Vector3Normalize(movement);

        movement = Vector3Scale(movement, speed * dt * 60.0f);

        player.Position = Vector3Add(player.Position, movement);

        player.stepTimer -= dt;

        if (player.stepTimer <= 0 && !IsKeyDown(KEY_LEFT_SHIFT))
        {
            if (BlockBelowPlayer > 0 && BlockBelowPlayer < (int)BlockEntries.size())
            {
                Sound GrainyStep = SFX_StepGrainy;
                Sound StepSoft = SFX_StepSoft;
                
                if (player.variant)
                {
                    GrainyStep = SFX_StepGrainy2;
                    StepSoft = SFX_StepSoft2;
                }
                
                player.variant = !player.variant;
                
                Sound stepSounds[] = {(Sound){0}, SFX_StepHard, GrainyStep, StepSoft};
                
                SetSoundVolume(stepSounds[BlockEntries[BlockBelowPlayer].Soundtype], sfxVolume);
                PlaySound(stepSounds[BlockEntries[BlockBelowPlayer].Soundtype]);
            }

            player.stepTimer = IsKeyDown(KEY_LEFT_CONTROL) ? 0.3f : 0.45f;
        }
    }
    else
    {
        player.stepTimer = 0.0f;
    }

    player.Position = Vector3Add(player.Position, Vector3Scale(player.Velocity, dt));
    
    if (player.Position.y < -1000) player.Position.y = 1000;
    
    PlayerCollision(player, chunks);
    
    int BlockAbovePlayer = BlockAtPos(
        (Vector3){
            player.Position.x,
            player.Position.y + player.Size.y + 0.1f,
            player.Position.z
        },
        chunks
    );
    
    if (BlockAbovePlayer > 0 && BlockEntries[BlockAbovePlayer].solid)
    {
        if (player.Velocity.y > 0) player.Velocity.y = 0;
    }
    
    if(player.health < lasthealth) {
        SetSoundVolume(SFX_Hurt, sfxVolume);
        PlaySound(SFX_Hurt);
    }
    
    return player;
    
    if(multiplayer) {
        //PlayerPacket out = {player.Position.x, player.Position.y, player.Position.z, player.Yaw, player.Pitch};
        //sock.send(reinterpret_cast<const std::byte*>(&out), sizeof(PlayerPacket));
    }
}