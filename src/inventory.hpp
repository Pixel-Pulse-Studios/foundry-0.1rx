struct ItemEntry {
    // Textures
    int TextureNames[6];
    
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
};

struct ItemSlot {
    bool isBlock;
    int id;
    int count;
};