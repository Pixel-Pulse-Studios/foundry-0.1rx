inline void PushAtlasUV(std::vector<float>& texcoords, int tile)
{
    const float tileSize = 1.0f / 8.0f;

    int tx = tile % 8;
    int ty = tile / 8;

    float u0 = tx * tileSize;
    float v0 = ty * tileSize;
    float u1 = u0 + tileSize;
    float v1 = v0 + tileSize;

    texcoords.insert(texcoords.end(), {
        u0, v1,
        u1, v1,
        u1, v0,
        u0, v0
    });
}

void BuildChunkMesh(Chunk& chunk, bool translucent, vector<Chunk>& chunks,float dayTime = 0.5f)
{
    if ((!translucent && chunk.ChunkMesh.vertexCount > 0) ||
        (translucent && chunk.TranslucentChunkMesh.vertexCount > 0))
    {
        if (translucent) {
            UnloadMesh(chunk.TranslucentChunkMesh);
        } else {
            UnloadMesh(chunk.ChunkMesh);
        }
    }
    
    chunk.Properties = UpdateLight(chunk,chunks,dayTime);
    TraceLog(LOG_INFO, "dayTime=%.2f light[100]=%d", dayTime, chunk.Properties[100].lights);

    std::vector<float> vertices;
    std::vector<unsigned short> indices;
    std::vector<float> texcoords;
    std::vector<unsigned char> colors;

    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (int y = 0; y < CHUNK_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                int index = x +
                            z * CHUNK_WIDTH +
                            y * CHUNK_WIDTH * CHUNK_WIDTH;

                int block = chunk.blocks[index];

                if (block == 0) continue;
                if (!translucent && BlockEntries[block].translucent) continue;
                if (translucent && !BlockEntries[block].translucent) continue;

                int tileID = BlockEntries[block].TextureSrc[0];
                int SidetileID = 0;
                if(BlockEntries[block].TextureSrc[1]) SidetileID = BlockEntries[block].TextureSrc[1];
                float transparency = BlockEntries[block].transparency;
                unsigned char alpha = (unsigned char)((1.0f - transparency) * 255.0f);
                unsigned char lightLevel = chunk.Properties[index].lights;
                unsigned char rgblight = lightLevel;
                
                // TOP
                if (!IsFaceCovered(chunk, x, y, z, 5, chunks))
                {
                    unsigned short n = (unsigned short)(vertices.size() / 3);

                    vertices.insert(vertices.end(), {
                        (float)x,   (float)y+1, (float)z+1,
                        (float)x+1, (float)y+1, (float)z+1,
                        (float)x+1, (float)y+1, (float)z,
                        (float)x,   (float)y+1, (float)z
                    });

                    indices.insert(indices.end(), {
                        n, (unsigned short)(n+1), (unsigned short)(n+2),
                        n, (unsigned short)(n+2), (unsigned short)(n+3)
                    });
                    for (int i = 0; i < 4; i++) {
                        colors.push_back(rgblight * 28);
                        colors.push_back(rgblight * 28);
                        colors.push_back(rgblight * 28);
                        colors.push_back(alpha);
                    }
                    PushAtlasUV(texcoords, tileID);
                }

                // BOTTOM
                if (!IsFaceCovered(chunk, x, y, z, 6, chunks))
                {
                    unsigned short n = (unsigned short)(vertices.size() / 3);

                    vertices.insert(vertices.end(), {
                        (float)x,   (float)y, (float)z,
                        (float)x+1, (float)y, (float)z,
                        (float)x+1, (float)y, (float)z+1,
                        (float)x,   (float)y, (float)z+1
                    });

                    indices.insert(indices.end(), {
                        n, (unsigned short)(n+1), (unsigned short)(n+2),
                        n, (unsigned short)(n+2), (unsigned short)(n+3)
                    });
                    for (int i = 0; i < 4; i++) {
                        colors.push_back(rgblight * 28);
                        colors.push_back(rgblight * 28);
                        colors.push_back(rgblight * 28);
                        colors.push_back(alpha);
                    }
                    PushAtlasUV(texcoords, tileID);
                }

                // FRONT
                if (!IsFaceCovered(chunk, x, y, z, 1, chunks))
                {
                    unsigned short n = (unsigned short)(vertices.size() / 3);

                    vertices.insert(vertices.end(), {
                        (float)x,   (float)y,   (float)z+1,
                        (float)x+1, (float)y,   (float)z+1,
                        (float)x+1, (float)y+1, (float)z+1,
                        (float)x,   (float)y+1, (float)z+1
                    });

                    indices.insert(indices.end(), {
                        n, (unsigned short)(n+1), (unsigned short)(n+2),
                        n, (unsigned short)(n+2), (unsigned short)(n+3)
                    });
                    for (int i = 0; i < 4; i++) {
                        colors.push_back(rgblight * 24);
                        colors.push_back(rgblight * 24);
                        colors.push_back(rgblight * 24);
                        colors.push_back(alpha);
                    }
                    if(SidetileID) { PushAtlasUV(texcoords, SidetileID); } else
                    PushAtlasUV(texcoords, tileID);
                }

                // BACK
                if (!IsFaceCovered(chunk, x, y, z, 2, chunks))
                {
                    unsigned short n = (unsigned short)(vertices.size() / 3);

                    vertices.insert(vertices.end(), {
                        (float)x+1, (float)y,   (float)z,
                        (float)x,   (float)y,   (float)z,
                        (float)x,   (float)y+1, (float)z,
                        (float)x+1, (float)y+1, (float)z
                    });

                    indices.insert(indices.end(), {
                        n, (unsigned short)(n+1), (unsigned short)(n+2),
                        n, (unsigned short)(n+2), (unsigned short)(n+3)
                    });
                    for (int i = 0; i < 4; i++) {
                        colors.push_back(rgblight * 22);
                        colors.push_back(rgblight * 22);
                        colors.push_back(rgblight * 22);
                        colors.push_back(alpha);
                    }
                    if(SidetileID) { PushAtlasUV(texcoords, SidetileID); } else
                    PushAtlasUV(texcoords, tileID);
                }

                // RIGHT
                if (!IsFaceCovered(chunk, x, y, z, 3, chunks))
                {
                    unsigned short n = (unsigned short)(vertices.size() / 3);

                    vertices.insert(vertices.end(), {
                        (float)x+1, (float)y,   (float)z+1,
                        (float)x+1, (float)y,   (float)z,
                        (float)x+1, (float)y+1, (float)z,
                        (float)x+1, (float)y+1, (float)z+1
                    });

                    indices.insert(indices.end(), {
                        n, (unsigned short)(n+1), (unsigned short)(n+2),
                        n, (unsigned short)(n+2), (unsigned short)(n+3)
                    });
                    for (int i = 0; i < 4; i++) {
                        colors.push_back(rgblight * 17);
                        colors.push_back(rgblight * 17);
                        colors.push_back(rgblight * 17);
                        colors.push_back(alpha);
                    }
                    if(SidetileID) { PushAtlasUV(texcoords, SidetileID); } else
                    PushAtlasUV(texcoords, tileID);
                }

                // LEFT
                if (!IsFaceCovered(chunk, x, y, z, 4, chunks))
                {
                    unsigned short n = (unsigned short)(vertices.size() / 3);

                    vertices.insert(vertices.end(), {
                        (float)x, (float)y,   (float)z,
                        (float)x, (float)y,   (float)z+1,
                        (float)x, (float)y+1, (float)z+1,
                        (float)x, (float)y+1, (float)z
                    });

                    indices.insert(indices.end(), {
                        n, (unsigned short)(n+1), (unsigned short)(n+2),
                        n, (unsigned short)(n+2), (unsigned short)(n+3)
                    });
                    for (int i = 0; i < 4; i++) {
                        colors.push_back(rgblight * 17);
                        colors.push_back(rgblight * 17);
                        colors.push_back(rgblight * 17);
                        colors.push_back(alpha);
                    }
                    if(SidetileID) { PushAtlasUV(texcoords, SidetileID); } else
                    PushAtlasUV(texcoords, tileID);
                }
            }
        }
    }

    Mesh mesh = {0};

    mesh.vertexCount   = vertices.size() / 3;
    mesh.triangleCount = indices.size() / 3;

    mesh.vertices  = (float*)MemAlloc(vertices.size() * sizeof(float));
    mesh.indices   = (unsigned short*)MemAlloc(indices.size() * sizeof(unsigned short));
    mesh.texcoords = (float*)MemAlloc(texcoords.size() * sizeof(float));
    mesh.colors = (unsigned char*)MemAlloc(colors.size() * sizeof(unsigned char));
    
    memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));
    memcpy(mesh.texcoords, texcoords.data(), texcoords.size() * sizeof(float));
    memcpy(mesh.colors, colors.data(), colors.size() * sizeof(unsigned char));
    UploadMesh(&mesh, false);

    if (translucent)
        chunk.TranslucentChunkMesh = mesh;
    else
        chunk.ChunkMesh = mesh;
}

void RebuildNeighborChunks(Chunk& chunk, vector<Chunk>& chunks, float dayTime = 0.5f) {
    int cx = chunk.ChunkX;
    int cz = chunk.ChunkZ;

    int neighbors[4][2] = {{cx+1,cz},{cx-1,cz},{cx,cz+1},{cx,cz-1}};

    for (auto& n : neighbors) {
        for (Chunk& neighbor : chunks) {
            if (neighbor.ChunkX == n[0] && neighbor.ChunkZ == n[1]) {
                UpdateLight(neighbor, chunks, dayTime);
                BuildChunkMesh(neighbor, false, chunks, dayTime);
                BuildChunkMesh(neighbor, true, chunks, dayTime);
                break;
            }
        }
    }
}

void BuildChunkMeshes(vector<Chunk>& chunks,float dayTime = 0.5) {
    for(int x = 0; x < WORLD_SIZE; x++) {
            for(int z = 0; z < WORLD_SIZE; z++) {
                int index = x + z * WORLD_SIZE;
                Chunk& chunk = chunks[index];
                BuildChunkMesh(chunk,false,chunks,dayTime);
                BuildChunkMesh(chunk,true,chunks,dayTime);
        }
    }
}