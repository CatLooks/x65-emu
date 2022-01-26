// -- component assembly -- //

// devices
bool sram;
bt bufbyte;
SDL_Joystick* joy1;
SDL_Joystick* joy2;
CPU cpu;
GPU gpu;

// memory
bt ram[0x4000];
bt sav[0x10000];
bt rom[0x100000];
bt banks[8] {0};
bt sbank;

// cpu map
void set(wt addr, bt data) {
    // RAM
    if (addr < 0x4000) {
        //printf("W RAM %04X = %02X\n", addr, data);
        ram[addr] = data;
        return;
    };

    // ROM
    if (addr >= 0x8000) {
        //printf("RAM %04X = %02X\n", addr, data);
        return;
    };

    // SRAM
    if (addr >= 0x6000) {
        if (sram) sav[(sbank << 13) | (addr & 0x1FFF)] = data;
        return;
    };

    // APU registers
    if (addr >= 0x5000) {
        if (addr < 0x5040) {
            bt channel = (addr & 0xE) >> 1;
            switch (addr & 0x31) {
                case 0x00:
                APU::mixer.channel(channel).freqL(data);
                break;
                case 0x01:
                APU::mixer.channel(channel).freqH(data);
                break;
                case 0x10:
                APU::mixer.channel(channel).volL(data);
                break;
                case 0x11:
                APU::mixer.channel(channel).volR(data);
                break;
                case 0x20:
                APU::mixer.channel(channel).loopL(data);
                break;
                case 0x21:
                APU::mixer.channel(channel).loopH(data);
                break;
                case 0x30:
                case 0x31:
                APU::mixer.channel(channel).wave(data);
                break;
            };
        } else if (addr & 1) {
            for (int i = 0; i < 8; i++) {
                if (data & (1 << (i ^ 7)))
                    APU::mixer.channel(i).enable(true);
            };
        } else for (int i = 0; i < 8; i++) {
            APU::mixer.channel(i).enable(data & (1 << (i ^ 7)));
        };
        return;
    };

    // registers
    switch (addr) {
        // GPU
        case 0x4000:
        case 0x4001:
        gpu.write(data);
        break;
        case 0x4002:
        gpu.control(data);
        break;
        case 0x4003:
        gpu.room(data);
        break;
        case 0x4004:
        case 0x4005:
        gpu.vramAddr(data);
        break;
        case 0x4006:
        case 0x4007:
        gpu.spriteAddr(data);
        break;
        case 0x4008:
        case 0x4009:
        gpu.scroll(data, 0);
        break;
        case 0x400A:
        case 0x400B:
        gpu.scroll(data, 1);
        break;
        case 0x400C:
        case 0x400D:
        gpu.scroll(data, 2);
        break;
        case 0x400E:
        case 0x400F:
        gpu.scroll(data, 3);
        break;

        // Banks
        case 0x4010:
        banks[0] = data;
        break;
        case 0x4011:
        banks[1] = data;
        break;
        case 0x4012:
        banks[2] = data;
        break;
        case 0x4013:
        banks[3] = data;
        break;
        case 0x4014:
        banks[4] = data;
        break;
        case 0x4015:
        banks[5] = data;
        break;
        case 0x4016:
        banks[6] = data;
        break;
        case 0x4017:
        banks[7] = data;
        break;
        case 0x4018:
        sbank = data & 0x7;
        break;

        // Debug
        case 0x4FFC:
        bufbyte = data;
        break;
        case 0x4FFD:
        printf("%04X", bufbyte | (data << 8));
        break;
        case 0x4FFE:
        printf("%02X", data);
        break;
        case 0x4FFF:
        putchar(data);
        break;
    };
};
bt get(wt addr) {
    // RAM
    if (addr < 0x4000) {
        //printf("R RAM %04X = %02X\n", addr, ram[addr]);
        return ram[addr];
    };

    // ROM
    if (addr >= 0x8000) {
        //printf("ROM %05X = %02X (Bank = %02X, Addr = %03X)\n", (banks[(addr >> 12) & 7] << 12) | (addr & 0xFFF), rom[(banks[(addr >> 12) & 7] << 12) | (addr & 0xFFF)], banks[(addr >> 12) & 7], addr & 0xFFF);
        return rom[(banks[(addr >> 12) & 7] << 12) | (addr & 0xFFF)];
    };

    // SRAM
    if (addr >= 0x6000) {
        return sram ? sav[(sbank << 13) | (addr & 0x1FFF)] : 0;
    };

    // registers
    switch (addr) {
        // GPU
        case 0x4000:
        case 0x4001:
        return gpu.read();

        // Joystick
        case 0x5000:
        return gpu.keys1() & 0xFF;
        case 0x5001:
        return gpu.keys1() >> 8;
        case 0x5002:
        return gpu.keys2() & 0xFF;
        case 0x5003:
        return gpu.keys2() >> 8;
    };

    return 0;
};
void act() {
    tick(cpu);
};
