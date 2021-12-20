// -- engine -- //

// key codes
#define KEY_DOWN   0x001
#define KEY_UP     0x002
#define KEY_RIGHT  0x004
#define KEY_LEFT   0x008
#define KEY_Y      0x010
#define KEY_X      0x020
#define KEY_B      0x040
#define KEY_A      0x080
#define KEY_R      0x100
#define KEY_L      0x200
#define KEY_START  0x400
#define KEY_SELECT 0x800

// gpu object
class GPU {
    public:
    // window creator
    bool create(st name, int width, int height) {
        // initialize sdl
        if (SDL_Init(SDL_INIT_EVERYTHING)) return false;

        // create window
        m_win = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
        if (m_win == null) return false;

        // create surfaces
        m_sur = SDL_CreateRGBSurfaceWithFormat(0, 16384, 8, 8, SDL_PIXELFORMAT_INDEX8);
        m_buf = SDL_CreateRGBSurfaceWithFormat(0, 320, 240, 32, SDL_PIXELFORMAT_ARGB32);
        m_scr = SDL_GetWindowSurface(m_win);
        if (m_sur == null) return false;
        if (m_buf == null) return false;

        // create palettes
        for (int i = 0; i < 16; i++)
            m_pal[i] = SDL_AllocPalette(16);

        // power cycle
        power();
        nmib = false;
        head = 0;

        // success
        m_keystate = SDL_GetKeyboardState(null);
        m_run = true;
        return true;
    };

    // destructor
    ~GPU () {
        SDL_DestroyWindow(m_win);
        for (int i = 0; i < 16; i++)
            SDL_FreePalette(m_pal[i]);
        SDL_Quit();
    };

    // power cycle
    void power() {
        // randomize initial state
        for (int i = 0; i < 16; i++) {
            SDL_Color colors[16];
            for (int j = 0; j < 16; j++) {
                colors[j].r = 0x0;
                colors[j].g = 0x0;
                colors[j].b = 0x0;
                colors[j].a = 0x0;
            };
            SDL_SetPaletteColors(m_pal[i], colors, 0, 16);
        };
        for (int l = 0; l < 2; l++) {
            for (int r = 0; r < 4; r++) {
                for (int i = 0; i < 1200; i++) {
                    layers[l].data[r][i].p1 = 0x0;
                    layers[l].data[r][i].p2 = 0x0;
                };
            };
        };
        for (int i = 0; i < 128; i++) {
            sprites[i].p1 = 0x0;
            sprites[i].p2 = 0x0;
            sprites[i].p3 = 0x0;
            sprites[i].p4 = 0x0;
            sprites[i].p5 = 0x0;
            sprites[i].p6 = 0x0;
        };
    };

    // event handler
    void events() {
        SDL_Event evt;

        // handler loop
        while (SDL_PollEvent(&evt)) {
            // quit event
            if (evt.type == SDL_QUIT) {
                m_run = false;
                return;
            };
        };
    };

    // window update
    void update() {
        // update key states
        SDL_PumpEvents();
        m_keys = 0;

        m_keys |= (m_keystate[SDL_SCANCODE_DOWN]) << 0;
        m_keys |= (m_keystate[SDL_SCANCODE_UP]) << 1;
        m_keys |= (m_keystate[SDL_SCANCODE_RIGHT]) << 2;
        m_keys |= (m_keystate[SDL_SCANCODE_LEFT]) << 3;
        m_keys |= (m_keystate[SDL_SCANCODE_A]) << 4;
        m_keys |= (m_keystate[SDL_SCANCODE_S]) << 5;
        m_keys |= (m_keystate[SDL_SCANCODE_Z]) << 6;
        m_keys |= (m_keystate[SDL_SCANCODE_X]) << 7;
        m_keys |= (m_keystate[SDL_SCANCODE_W]) << 8;
        m_keys |= (m_keystate[SDL_SCANCODE_Q]) << 9;
        m_keys |= (m_keystate[SDL_SCANCODE_RETURN]) << 10;
        m_keys |= (m_keystate[SDL_SCANCODE_SPACE]) << 11;
    };

    // window render
    void render(inpf get) {
        // sort sprites by layer
        vec<Sprite*>sorted[3];
        for (int i = 0; i < 128; i++) {
            Sprite& spr = sprites[i];
            if (spr.layer() == 0)
                sorted[0].push_back(&spr);
            else if (spr.layer() == 1)
                sorted[1].push_back(&spr);
            else
                sorted[2].push_back(&spr);
        };

        // dma sprite data
        if (sprb) {
            for (int i = 0; i < 128; i++) {
                sprites[i].p3 = get(saddr + i * 2 + 0x000);
                sprites[i].p4 = get(saddr + i * 2 + 0x001);
                sprites[i].p5 = get(saddr + i * 2 + 0x100);
                sprites[i].p6 = get(saddr + i * 2 + 0x101);
                sprites[i].p1 = get(saddr + i * 2 + 0x200);
                sprites[i].p2 = get(saddr + i * 2 + 0x201);

                /*printf("Sprite $%02X:\n", i);
                printf("- Tile:    %03X\n", sprites[i].id());
                printf("- Layer:   %d\n", sprites[i].layer());
                printf("- Palette: %X\n", sprites[i].palette());
                printf("- Pos: %d %d\n", sprites[i].x(), sprites[i].y());*/
            };
        };

        // check scrolls
        layers[0].scrollx %= 320;
        layers[0].scrolly %= 240;
        layers[1].scrollx %= 320;
        layers[1].scrolly %= 240;

        // render screen
        SDL_Surface* tgt = SDL_CreateRGBSurface(0, 320, 240, m_scr->format->BitsPerPixel, m_scr->format->Rmask, m_scr->format->Gmask, m_scr->format->Bmask, m_scr->format->Amask);
        SDL_FillRect(tgt, null, SDL_MapRGBA(m_scr->format, m_pal[0]->colors[0].r, m_pal[0]->colors[0].g, m_pal[0]->colors[0].b, m_pal[0]->colors[0].a));
        if (sprb) {
            renderSprites(sorted[0]);
            SDL_BlitSurface(m_buf, null, tgt, null);
        };
        if (lay1) {
            renderLayer(layers[0]);
            SDL_BlitSurface(m_buf, null, tgt, null);
        };
        if (sprb) {
            renderSprites(sorted[1]);
            SDL_BlitSurface(m_buf, null, tgt, null);
        };
        if (lay2) {
            renderLayer(layers[1]);
            SDL_BlitSurface(m_buf, null, tgt, null);
        };
        if (sprb) {
            renderSprites(sorted[2]);
            SDL_BlitSurface(m_buf, null, tgt, null);
        };
        SDL_BlitScaled(tgt, null, m_scr, null);
        SDL_UpdateWindowSurface(m_win);
        SDL_FreeSurface(tgt);
    };

    // layer render
    void renderLayer(Layer& layer) {
        int sx = layer.scrollx + layer.roomx * 320;
        int sy = layer.scrolly + layer.roomy * 240;

        for (int y = 0; y < 31; y++) {
            for (int x = 0; x < 41; x++) {
                int tx = (x + (sx >> 3)) % 80;
                int ty = (y + (sy >> 3)) % 60;
                Tile& tile = layer.data[toRoom(tx, ty)][toIndex(tx, ty)];

                SDL_Rect src = {tile.id() << 3, 0, 8, 8};
                SDL_Rect dst = {(x << 3) - (sx & 7), (y << 3) - (sy & 7), 8, 8};

                SDL_SetSurfacePalette(m_sur, m_pal[tile.palette()]);
                SDL_BlitSurface(m_sur, &src, m_buf, &dst);
            };
        };
    };

    // sprites render
    void renderSprites(vec<Sprite*> sprites) {
        for (Sprite* spr : sprites) {
            SDL_Rect src = {(spr->id() | 0x400) << 3, 0, 8, 8};
            SDL_Rect dst = {spr->x(), spr->y(), 8, 8};

            SDL_SetSurfacePalette(m_sur, m_pal[spr->palette()]);
            SDL_BlitSurface(m_sur, &src, m_buf, &dst);
        };
    };

    // fps capper
    void start() {
        m_timer = SDL_GetTicks();
    };
    void stop(proc action) {
        /*int delta = SDL_GetTicks() - m_timer;
        if (delta < 16)
            SDL_Delay(16 - delta);*/
        while (true) {
            int delta = SDL_GetTicks() - m_timer;
            action();
            if (delta >= 16)
                break;
        };
    };

    // is window active
    bool running() {
        return m_run;
    };
    // check nmi
    bool nmi() {
        return nmib;
    };

    // get key state
    wt keys() {
        return m_keys;
    };

    // get layer
    Layer& layer(bool id) {
        return layers[id];
    };
    // get sprite reference
    Sprite& sprite(bt id) {
        return sprites[id & 0x7F];
    };
    // get cgram reference
    SDL_Surface* cgram() {
        return m_sur;
    };
    // get palette color
    SDL_Color& palette(bt id) {
        return m_pal[id >> 4]->colors[id & 0xF];
    };

    // registers
    void spriteAddr(bt data) {
        saddr >>= 8;
        saddr |= data << 8;
    };
    void vramAddr(bt data) {
        vaddr >>= 8;
        vaddr |= data << 8;
    };
    void scroll(bt data, bt id) {
        switch (id) {
            case 0:
            layers[0].scrollx >>= 8;
            layers[0].scrollx |= data << 8;
            break;
            case 1:
            layers[0].scrolly >>= 8;
            layers[0].scrolly |= data << 8;
            break;
            case 2:
            layers[1].scrollx >>= 8;
            layers[1].scrollx |= data << 8;
            break;
            case 3:
            layers[1].scrolly >>= 8;
            layers[1].scrolly |= data << 8;
            break;
        };
    };

    // I/O
    void write(bt data) {
        wt addr = vaddr;

        if (addr >= 0x8000) {
            switch (head & 0x3) {
                case 0:
                vaddr = (vaddr & 0xF800) + mod((vaddr & 0x7FF) + 1, 1200);
                break;
                case 1:
                vaddr = (vaddr & 0xF800) + mod((vaddr & 0x7FF) - 1, 1200);
                break;
                case 2: {
                    if ((vaddr & 0x7FF) == 0x4AF) {
                        vaddr = vaddr & 0xF800;
                        break;
                    };
                    vaddr = (vaddr & 0xF800) + mod((vaddr & 0x7FF) + 40, 1200);
                    if (addr > vaddr) vaddr++;
                };
                break;
                case 3: {
                    if ((vaddr & 0x7FF) == 0) {
                        vaddr = (vaddr & 0xF800) | 0x4AF;
                        break;
                    };
                    vaddr = (vaddr & 0xF800) + mod((vaddr & 0x7FF) - 40, 1200);
                    if (addr < vaddr) vaddr--;
                };
                break;
            };

            bt block = (addr >> 11) & 0xF;
            wt offset = addr & 0x7FF;

            if (offset < 1200) {
                wt xc = offset % 40;
                wt yc = offset / 40;

                if (block >> 3)
                    layers[(block >> 2) & 1].data[block & 3][yc * 40 + xc].p2 = data;
                else
                    layers[(block >> 2) & 1].data[block & 3][yc * 40 + xc].p1 = data;
            };
            return;
        };
        vaddr++;

        if (addr < 0x200) {
            bt id = addr >> 1;

            if (addr & 1) {
                palette(id).b = (data >> 4) * 0x11;
                palette(id).a = (data & 15) * 0x11;
            } else {
                palette(id).r = (data >> 4) * 0x11;
                palette(id).g = (data & 15) * 0x11;
            };
        };
    };
    bt read() {
        wt addr = vaddr;

        if (addr >= 0x8000) {
            switch (head & 0x3) {
                case 0:
                vaddr = (vaddr & 0xF800) + mod((vaddr & 0x7FF) + 1, 1200);
                break;
                case 1:
                vaddr = (vaddr & 0xF800) + mod((vaddr & 0x7FF) - 1, 1200);
                break;
                case 2: {
                    if ((vaddr & 0x7FF) == 0x4AF) {
                        vaddr = vaddr & 0xF800;
                        break;
                    };
                    vaddr = (vaddr & 0xF800) + mod((vaddr & 0x7FF) + 40, 1200);
                    if (addr > vaddr) vaddr++;
                };
                break;
                case 3: {
                    if ((vaddr & 0x7FF) == 0) {
                        vaddr = (vaddr & 0xF800) | 0x4AF;
                        break;
                    };
                    vaddr = (vaddr & 0xF800) + mod((vaddr & 0x7FF) - 40, 1200);
                    if (addr < vaddr) vaddr--;
                };
                break;
            };

            bt block = (addr >> 11) & 0xF;
            wt offset = addr & 0x7FF;

            if (offset < 1200) {
                wt xc = offset % 40;
                wt yc = offset / 40;

                if (block >> 3)
                    return layers[(block >> 2) & 1].data[block & 3][yc * 40 + xc].p2;
                return layers[(block >> 2) & 1].data[block & 3][yc * 40 + xc].p1;
            };
            return 0;
        };
        vaddr++;

        return 0;
    };
    void control(bt data) {
        nmib = data & 0x80;
        sprb = data & 0x40;
        lay1 = data & 0x20;
        lay2 = data & 0x10;
        head = data & 0x3;
    };
    void room(bt data) {
        layers[0].roomy = data & 0x8;
        layers[0].roomx = data & 0x4;
        layers[1].roomy = data & 0x2;
        layers[1].roomx = data & 0x1;
    };

    private:
    // window control
    SDL_Palette*  m_pal[16];
    SDL_Surface*  m_scr;
    SDL_Surface*  m_buf;
    SDL_Surface*  m_sur;
    SDL_Window*   m_win;
    const Uint8* m_keystate;
    bool m_run = false;
    dt m_timer = 0;
    wt m_keys = 0;

    // gpu data
    Layer layers[2];
    Sprite sprites[128];
    wt vaddr;
    wt saddr;
    bt head;
    bool nmib;
    bool sprb;
    bool lay1;
    bool lay2;
};
