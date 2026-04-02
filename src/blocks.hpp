enum SoundType {
    SOUND_NONE = 0,
    SOUND_HARD = 1,
    SOUND_GRAINY = 2,
    SOUND_SOFT = 3,
};

Sound SFX_BreakHard;
Sound SFX_BreakGrainy;
Sound SFX_BreakSoft;

Sound SFX_PlaceHard;
Sound SFX_PlaceGrainy;
Sound SFX_PlaceSoft;

Sound SFX_StepHard;
Sound SFX_StepGrainy;
Sound SFX_StepGrainy2;
Sound SFX_StepSoft;
Sound SFX_StepSoft2;

typedef struct {
    // Textures
    Texture2D Textures[6];
    int TextureSrc[6];
    
    //Sound
    int Soundtype;
    
    // Render Properties
    float transparency;
    bool translucent;
    bool flat;
    
    // Gameplay Properties
    bool solid;
    float hardness;
    unsigned char lightLevel;
    std::string name;
    bool damaging;
    float damage;
} BlockEntry;

Texture2D GlobalAtlas;

vector<BlockEntry> BlockEntries;

void Note(const char* note) {
    return;
}

void NewBlock(std::vector<int> Textures, float transparency, bool translucent, bool flat, bool solid, float hardness, uint8_t light, std::string name, int soundtype = 0) {
    Note("New Block Function");
    BlockEntry newBlock;
    
    // Add textures to the new block entry
    for (int i = 0; i < Textures.size(); ++i) {
        newBlock.TextureSrc[i] = Textures[i];
    }

    // Set Render Properties
    newBlock.transparency = transparency;
    newBlock.translucent = translucent;
    if(newBlock.transparency != 0) newBlock.translucent = true;
    newBlock.flat = flat;
    
    newBlock.Soundtype = soundtype;
    
    // Set Gameplay Properties
    newBlock.solid = solid;
    newBlock.hardness = hardness;
    newBlock.lightLevel = light;
    newBlock.name = name;

    // Add to BlockEntries
    BlockEntries.push_back(newBlock);
}
