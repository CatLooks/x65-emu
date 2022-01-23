// -- core file -- //

// include libraries
#include <SDL2/SDL.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <math.h>

// define types
#define null __null
#define vec std::vector
typedef unsigned short wt;
typedef unsigned char bt;
typedef unsigned int dt;
typedef const char* st;
typedef void (*proc)();
typedef char* mt;

// include project
#include "types.h"
#include "macros.h"
#include "x65-cpu.h"
using namespace x65;
#include "x65-gpu.h"
#include "x65-apu.h"

// device assembly
#include "asm.h"

// file manager
#include "file.h"

// program entry
int main(int argc, mt* argv) {
    if (argc < 2)
        return 0;

    // initialize sdl
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        printf(" - %s\n", SDL_GetError());
        return 1;
    };

    // save file name
    char filename[512];
    sprintf(filename, "%s.sav", argv[1]);

    // open rom
    File file = loadFile(argv[1]);
    if (!file.valid) {
        printf(" - Failed to open %s\n", argv[1]);
        return 2;
    };

    // init audio
    if (!APU::create(sampleRate)) {
        printf(" - %s\n", SDL_GetError());
        return 3;
    };

    // try to open joystick
    if (SDL_NumJoysticks() > 0) {
        joy1 = SDL_JoystickOpen(0);
        joy2 = SDL_JoystickOpen(1);
        gpu.setJoystickUse(joy1 || joy2);
    };

    // init window
    if (!gpu.create("X65", 320 * 2, 240 * 2)) {
        printf(" - %s\n", SDL_GetError());
        return 4;
    };

    // randomize memory state
    for (int i = 0; i < 0x4000; i++)
        ram[i] = rand() & 0xFF;
    cpu.a = rand();
    cpu.b = rand();
    cpu.x = rand();
    cpu.y = rand();

    // parse rom
    int errlevel = loadROM(file.data);
    if (errlevel) {
        // load error rom
        File errc = loadFile(rootFile("error.x65"));
        if (!errc.valid) {
            printf(" - Failed to open error cart\n");
            return 5;
        };

        int ferr = loadROM(errc.data);
        if (ferr)
            return ferr;

        ram[0x00] = errlevel;
    };

    // load save file
    if (sram) {
        File save = loadFile(filename);
        if (save.valid) {
            if (save.data.size() == 0x10000) {
                for (dt i = 0; i < save.data.size(); i++) {
                    sav[i] = save.data[i];
                };
            } else {
                printf(" - Save file should be 64K long\n");
            };
        };
    };

    // cpu mapping
    cpu.set = &set;
    cpu.get = &get;

    // initial reset
    vectorRST(cpu);

    // main loop
    while (gpu.running()) {
        gpu.start();
        gpu.events(cpu, joy1, joy2);
        gpu.update(joy1, joy2);

        if (gpu.nmi()) {
            vectorNMI(cpu);
        };

        gpu.render(get);
        gpu.stop(act);
    };

    // close joystick
    if (joy1)
        SDL_JoystickClose(joy1);
    if (joy2)
        SDL_JoystickClose(joy2);

    // save SRAM
    if (sram) {
        File sf;
        sf.name = filename;
        sf.valid = true;
        for (int i = 0; i < 0x10000; i++)
            sf.data.push_back(sav[i]);
        saveFile(sf);
    };

    // success
    SDL_CloseAudio();
    return 0;
};
