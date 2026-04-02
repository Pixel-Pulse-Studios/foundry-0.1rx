void RenderTexturedFace(Texture2D Texture, Color Tint, Vector3 Pos, Vector2 Size, Vector3 Rotate) {
    rlSetTexture(Texture.id);
    rlColor4ub(Tint.r, Tint.g, Tint.b, Tint.a);
    
    float halfWidth = Size.x * 0.5f;
    float halfHeight = Size.y * 0.5f;
    
    Vector3 corners[4] = {
        (Vector3){ -halfWidth, -halfHeight, 0 },
        (Vector3){ -halfWidth,  halfHeight, 0 },
        (Vector3){  halfWidth,  halfHeight, 0 },
        (Vector3){  halfWidth, -halfHeight, 0 }
    };
    
    Matrix rotMat = MatrixRotateXYZ(Rotate);
    
    rlTexCoord2f(0.0f, 1.0f); Vector3 v0 = Vector3Transform(corners[0], rotMat); rlVertex3f(Pos.x + v0.x, Pos.y + v0.y, Pos.z + v0.z);
    rlTexCoord2f(0.0f, 0.0f); Vector3 v1 = Vector3Transform(corners[1], rotMat); rlVertex3f(Pos.x + v1.x, Pos.y + v1.y, Pos.z + v1.z);
    rlTexCoord2f(1.0f, 0.0f); Vector3 v2 = Vector3Transform(corners[2], rotMat); rlVertex3f(Pos.x + v2.x, Pos.y + v2.y, Pos.z + v2.z);
    rlTexCoord2f(1.0f, 1.0f); Vector3 v3 = Vector3Transform(corners[3], rotMat); rlVertex3f(Pos.x + v3.x, Pos.y + v3.y, Pos.z + v3.z);

    rlDisableTexture();
}

void RenderTexturedCube(Texture2D texture, Color tint, Vector3 position, Vector2 size, float cubeSize) {
    float halfSize = cubeSize * 0.25f;
    
    RenderTexturedFace(texture, tint, 
        (Vector3){position.x, position.y, position.z + halfSize}, 
        size, 
        (Vector3){0.0f, 180.0f * DEG2RAD, 0.0f});
    
    RenderTexturedFace(texture, tint, 
        (Vector3){position.x, position.y, position.z - halfSize}, 
        size, 
        (Vector3){0.0f, 0.0f, 0.0f});
    
    RenderTexturedFace(texture, tint, 
        (Vector3){position.x + halfSize, position.y, position.z}, 
        size, 
        (Vector3){0.0f, -90.0f * DEG2RAD, 0.0f});
    
    RenderTexturedFace(texture, tint, 
        (Vector3){position.x - halfSize, position.y, position.z}, 
        size, 
        (Vector3){0.0f, 90.0f * DEG2RAD, 0.0f});
    
    RenderTexturedFace(texture, tint, 
        (Vector3){position.x, position.y + halfSize, position.z}, 
        size, 
        (Vector3){90.0f * DEG2RAD, 0.0f, 0.0f});
    
    RenderTexturedFace(texture, tint, 
        (Vector3){position.x, position.y - halfSize, position.z}, 
        size, 
        (Vector3){-90.0f * DEG2RAD, 0.0f, 0.0f});
}

void RenderVoxel(const Chunk& chunk, int x, int y, int z, Texture2D texture, Color tint, Vector3 position, Vector2 size, float cubeSize) {
    float halfSize = cubeSize * 0.25f;
    
    if (!IsFaceCovered(chunk, x, y, z, 1)) RenderTexturedFace(texture, tint, 
        (Vector3){position.x, position.y, position.z + halfSize}, 
        size, 
        (Vector3){0.0f, 180.0f * DEG2RAD, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 2)) RenderTexturedFace(texture, tint, 
        (Vector3){position.x, position.y, position.z - halfSize}, 
        size, 
        (Vector3){0.0f, 0.0f, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 3)) RenderTexturedFace(texture, tint, 
        (Vector3){position.x + halfSize, position.y, position.z}, 
        size, 
        (Vector3){0.0f, -90.0f * DEG2RAD, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 4)) RenderTexturedFace(texture, tint, 
        (Vector3){position.x - halfSize, position.y, position.z}, 
        size, 
        (Vector3){0.0f, 90.0f * DEG2RAD, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 5)) RenderTexturedFace(texture, tint, 
        (Vector3){position.x, position.y + halfSize, position.z}, 
        size, 
        (Vector3){90.0f * DEG2RAD, 0.0f, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 6)) RenderTexturedFace(texture, tint, 
        (Vector3){position.x, position.y - halfSize, position.z}, 
        size, 
        (Vector3){-90.0f * DEG2RAD, 0.0f, 0.0f});
}

void RenderSidedVoxel(const Chunk& chunk, int x, int y, int z, Texture2D textures[6], Color tint, Vector3 position, Vector2 size, float cubeSize) {
    float halfSize = cubeSize * 0.25f;
    
    if (!IsFaceCovered(chunk, x, y, z, 1)) RenderTexturedFace(textures[0], tint, 
        (Vector3){position.x, position.y, position.z + halfSize}, 
        size, 
        (Vector3){0.0f, 180.0f * DEG2RAD, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 2)) RenderTexturedFace(textures[1], tint, 
        (Vector3){position.x, position.y, position.z - halfSize}, 
        size, 
        (Vector3){0.0f, 0.0f, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 3)) RenderTexturedFace(textures[2], tint, 
        (Vector3){position.x + halfSize, position.y, position.z}, 
        size, 
        (Vector3){0.0f, -90.0f * DEG2RAD, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 4)) RenderTexturedFace(textures[3], tint, 
        (Vector3){position.x - halfSize, position.y, position.z}, 
        size, 
        (Vector3){0.0f, 90.0f * DEG2RAD, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 5)) RenderTexturedFace(textures[4], tint, 
        (Vector3){position.x, position.y + halfSize, position.z}, 
        size, 
        (Vector3){90.0f * DEG2RAD, 0.0f, 0.0f});
    
    if (!IsFaceCovered(chunk, x, y, z, 6)) RenderTexturedFace(textures[5], tint, 
        (Vector3){position.x, position.y - halfSize, position.z}, 
        size, 
        (Vector3){-90.0f * DEG2RAD, 0.0f, 0.0f});
}

void DrawChunk(Chunk chunk) { 
    for (int x = 0; x < CHUNK_WIDTH; x++) {
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                for(int z = 0; z < CHUNK_WIDTH; z++) {
                    int index = x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH;
                    if (chunk.blocks[index] != 0) {
                        Vector3 pos = {(float)(x), (float)(y), (float)(z)};
                        if(!IsBlockCovered(chunk, x, y, z)) {
                            //if (BlockEntries[chunk.blocks[index]].TextureSrc[1].empty()) RenderVoxel(chunk, x, y, z, BlockEntries[chunk.blocks[index]].Textures[0], WHITE, pos, (Vector2){1, 1}, 2.0f);
                            //if (!BlockEntries[chunk.blocks[index]].TextureSrc[5].empty()) RenderSidedVoxel(chunk, x, y, z, BlockEntries[chunk.blocks[index]].Textures, WHITE, pos, (Vector2){1, 1}, 2.0f);
                            DrawCubeWires((Vector3){(float)x,(float)y,(float)z}, 1.0f, 1.0f, 1.0f, RED);
                        }
                    }
                }
            }
        }
}