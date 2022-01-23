// -- file manager -- //

// file object
struct File {
    bool valid = false;
    st name = null;
    vec<bt> data;
};

// open file
File loadFile(st filename) {
    // open file stream
    FILE* fp = fopen(filename, "rb");
    if (fp == null)
        return File();

    // read file contents
    File inst;
    while (true) {
        bt c = fgetc(fp);
        if (feof(fp))
            break;
        inst.data.push_back(c);
    };

    // sign file
    inst.name = filename;
    inst.valid = true;
    fclose(fp);
    return inst;
};

// save file
bool saveFile(File& file) {
    // check file
    if (!(file.valid && file.name))
        return false;

    // open file stream
    FILE* fp = fopen(file.name, "wb");
    if (fp == null)
        return false;

    // write file contents
    for (bt c : file.data)
        fputc(c, fp);

    // close file
    fclose(fp);
    return true;
};

// get filename in root directory
mt rootFile(mt path) {
    char buffer[256];
    GetModuleFileNameA(null, buffer, 256);

    int slash = 0xFF;
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == '\\' || buffer[i] == '/') {
            slash = i + 1;
        };
    };

    vec<char> res;
    for (int i = 0; i < slash; i++) {
        res.push_back(buffer[i]);
    };
    for (int i = 0; i < strlen(path); i++) {
        res.push_back(path[i]);
    };
    res.push_back(0);

    mt out = new char[res.size()];
    for (int i = 0; i < res.size(); i++)
        out[i] = res[i];

    return out;
};

// parse ROM
int loadROM(vec<bt>& data) {
    // check file size
    if (data.size() < 16) {
        printf(" - File is too small\n");
        return 10;
    };

    // check for signature
    if (data[0] != 'x' || data[1] != '6' || data[2] != '5' || data[3] != 0) {
        printf(" - Invalid signature\n");
        return 16;
    };

    // read header data
    wt prg = data[0x4] | data[0x5] << 8;
    bt dsd = data[0x6];
    sram = data[0x7];
    if (prg > 0x100) {
        printf(" - Too large PRG ROM\n");
        return 11;
    };
    if (dsd > 0x80) {
        printf(" - Too large DSD ROM\n");
        return 12;
    };
    int chr = data.size() - dsd * waveSize - prg * 0x1000 - 0x10;
    if (chr < 0) {
        printf(" - Incomplete ROM\n");
        return 13;
    };
    if (chr % 32) {
        printf(" - CHR ROM contains unfinished data\n");
        return 14;
    };
    if (chr > 0x10000) {
        printf(" - Too large CHR ROM\n");
        return 15;
    };

    // copy PRG ROM
    dt i;
    for (i = 0; i < prg * 0x1000; i++) {
        if (i + 0x10 >= data.size()) {
            printf(" - PRG ROM segment is missing\n");
        };
        rom[i] = data[i + 0x10];
    };

    // copy DSD ROM
    dt j;
    for (j = 0; j < dsd * waveSize; j++) {
        if (i + j + 0x10 >= data.size()) {
            printf(" - DSD ROM segment is missing\n");
        };
        waveBuffer[j] = data[i + j + 0x10];
    };

    // copy CHR ROM
    SDL_Surface* cgram = gpu.cgram();
    for (int t = 0; t < chr; t++) {
        int x = ((t & 3) << 1) + ((t >> 5) << 3);
        int y = (t >> 2) & 7;
        bt d = data[i + j + t + 0x10];

        SetPixel(cgram, x + 0, y, d >> 4);
        SetPixel(cgram, x + 1, y, d & 15);
    };

    // success
    return 0;
};
