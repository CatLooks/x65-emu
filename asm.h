// -- component assembly -- //

// devices
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
    };

    // ROM
    if (addr >= 0x8000) {
        //printf("RAM %04X = %02X\n", addr, data);
        return;
    };

    // SRAM
    if (addr >= 0x6000) {
        sav[(sbank << 13) | (addr & 0x1FFF)] = data;
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
        return sav[(sbank << 13) | (addr & 0x1FFF)];
    };

    // registers
    switch (addr) {
        // GPU
        case 0x4000:
        case 0x4001:
        return gpu.read();

        // Joystick
        case 0x401E:
        case 0x401F:
        return gpu.keys();
    };

    return 0;
};
void act() {
    tick(cpu);
};
