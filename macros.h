// -- macros -- //

// set surface pixel color
void SetPixel(SDL_Surface* surface, int x, int y, Uint8 data) {
    Uint8* target = (Uint8*)surface->pixels;
    target[surface->pitch * y + x] = data;
};

// convert layer coords to memory coords
bt toRoom(int x, int y) {
    return (x >= 40 ? 1 : 0) | (y >= 30 ? 2 : 0);
};
wt toIndex(int x, int y) {
    return (y % 30) * 40 + (x % 40);
};

// mod fix
int mod(int a, int b) {
    if (a < 0)
        return a % b + b;
    return a % b;
};
