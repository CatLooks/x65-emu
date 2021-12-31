// -- core file -- //

// include libraries
#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>

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
        return 1;
    };

    // init audio
    if (!APU::create(sampleRate)) {
        printf(" - %s\n", SDL_GetError());
        return 1;
    };

    // try to open joystick
    if (SDL_NumJoysticks() > 0) {
        joy1 = SDL_JoystickOpen(0);
        joy2 = SDL_JoystickOpen(1);
    };

    // init window
    if (!gpu.create("X65", 320 * 3, 240 * 3)) {
        printf(" - %s\n", SDL_GetError());
        return 1;
    };

    // parse rom
    if (!loadROM(file.data))
        return 1;

    // load save file
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
    File sram;
    sram.name = filename;
    sram.valid = true;
    for (int i = 0; i < 0x10000; i++)
        sram.data.push_back(sav[i]);

    saveFile(sram);

    // success
    SDL_CloseAudio();
    return 0;
};
