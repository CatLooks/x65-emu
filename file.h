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

// parse ROM
bool loadROM(vec<bt>& data) {
    // check file size
    if (data.size() < 16) {
        printf(" - File is too small\n");
        return false;
    };

    // check for signature
    if (data[0] != 'x' || data[1] != '6' || data[2] != '5' || data[3] != 0) {
        printf(" - Invalid signature\n");
        return false;
    };

    // read header data
    wt prg = data[0x4] | data[0x5] << 8;
    if (prg > 0x100) {
        printf("- Too large PRG ROM\n");
        return false;
    };
    int chr = data.size() - prg * 0x1000 - 0x10;
    if (chr < 0) {
        printf("- Incomplete PRG ROM\n");
        return false;
    };
    if (chr % 32) {
        printf("- CHR ROM contains unfinished data\n");
        return false;
    };
    if (chr > 0x10000) {
        printf("- Too large CHR ROM\n");
        return false;
    };

    // copy PRG ROM
    dt i = 0;
    for (i = 0; i < prg * 0x1000; i++) {
        if (i + 0x10 >= data.size()) {
            printf(" - PRG ROM segment is missing\n");
        };
        rom[i] = data[i + 0x10];
    };

    // copy CHR ROM
    SDL_Surface* cgram = gpu.cgram();
    for (int t = 0; t < chr; t++) {
        int x = ((t & 3) << 1) + ((t >> 5) << 3);
        int y = (t >> 2) & 7;
        bt d = data[i + t + 0x10];

        SetPixel(cgram, x + 0, y, d >> 4);
        SetPixel(cgram, x + 1, y, d & 15);
    };

    // success
    return true;
};
