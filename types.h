// -- types -- //

// background tile
struct Tile {
    bt p1, p2;

    wt id() {
        return p1 | (p2 & 0x3) << 8;
    };
    bt palette() {
        return p2 >> 4;
    };
};

// sprite tile
struct Sprite {
    bt p1, p2, p3, p4, p5, p6;

    wt id() {
        return p1 | (p2 & 0x3) << 8;
    };
    bt layer() {
        return (p2 >> 2) & 0x3;
    };
    bt palette() {
        return p2 >> 4;
    };
    short x() {
        return p3 | p4 << 8;
    };
    short y() {
        return p5 | p6 << 8;
    };
};

// background layer
struct Layer {
    Tile data[4][1200];
    wt scrollx;
    wt scrolly;
    bool roomx;
    bool roomy;
};
