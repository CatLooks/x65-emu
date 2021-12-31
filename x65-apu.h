// -- audio -- //

// constants
const int sampleRate = 44100;
const int sampleCount = 735;
const int sampleCost = 64;
const int waveSize = 256;

// square waveform
Uint8 waveSquare[waveSize] {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
Uint8 waveTriangle[waveSize] {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
};
Uint8 waveSaw[waveSize] {
    0x00, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0x44, 0x44, 0x55, 0x55, 0x66, 0x66, 0x77, 0x77,
	0x88, 0x88, 0x99, 0x99, 0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, 0xDD, 0xDD, 0xEE, 0xEE, 0xFF, 0xFF
};

// waveform buffer
Uint8 waveBuffer[32768];

// channel object
class Channel {
    public:
    // constructor
    Channel () {
        m_active = false;
        m_phase = 0.0;
        m_freq = 1.0;
        m_lvol = 0.0;
        m_rvol = 0.0;
    };
    // set frequency
    void freq(int value) {
        m_freq = value;
        m_phase = 0.0;
    };
    void freqL(Uint8 value) {
        m_freq = (m_freq & 0xFF00) | value;
        m_phase = 0.0;
    };
    void freqH(Uint8 value) {
        m_freq = (m_freq & 0x00FF) | (value << 8);
        m_phase = 0.0;
    };
    // set volume
    void volL(Uint8 value) {
        m_lvol = float(value) / 255.0;
    };
    void volR(Uint8 value) {
        m_rvol = float(value) / 255.0;
    };
    // set wave form
    void wave(Uint8 id) {
        m_wave = id & 0x7F;
        m_phase = 0.0;
    };
    // toggle state
    void enable(bool state) {
        m_active = state;
    };
    // get next sample
    Uint32 next(unsigned int rate) {
        // update phase
        m_phase += float(m_freq) / rate;
        if (m_phase >= 1.0) m_phase -= 1.0;

        // check for state
        if (!m_active)
            return 0;

        // return samples
        Uint16 full = waveBuffer[int(m_phase * waveSize) + waveSize * m_wave] * sampleCost;
        return Uint16(full * m_lvol) << 16 | Uint16(full * m_rvol);
    };

    private:
    bool m_active;
    float m_phase;
    float m_lvol;
    float m_rvol;
    short m_freq;
    bt m_wave;
};

// mixer object
class Mixer {
    public:
    // constructor
    Mixer () {
        m_enabled = false;
    };
    // set sample rate
    void rate(unsigned int value) {
        m_rate = value;
    };
    // toggle mixer
    void enable(bool state) {
        m_enabled = state;
    };
    // access mixer channel
    Channel& channel(int id) {
        return m_channels[id];
    };
    // get next sample
    Uint32 next() {
        // merge all samples
        Uint16 lvalue = 0;
        Uint16 rvalue = 0;

        for (int i = 0; i < 8; i++) {
            Uint32 sample = m_channels[i].next(m_rate);
            lvalue += sample & 0xFFFF;
            rvalue += sample >> 16;
        };
        return lvalue << 16 | rvalue;
    };

    private:
    Channel m_channels[8];
    unsigned int m_rate;
    bool m_enabled;
};

// apu object
namespace APU {
    Mixer mixer;

    // audio callback
    void callback(void* ign, Uint8* dst, int len) {
        Uint16* stream = (Uint16*)dst;

        // fill buffer
        for (unsigned int i = 0; i < len / sizeof(Uint16) / 2; i++) {
            Uint32 sample = mixer.next();
            stream[i * 2 + 0] = sample & 0xFFFF;
            stream[i * 2 + 1] = sample >> 16;
        };
    };

    // constructor
    bool create(unsigned int rate) {
        // create audio device
        SDL_AudioSpec dev;
        dev.callback = callback;
        dev.format = AUDIO_U16;
        dev.freq = sampleRate;
        dev.samples = sampleCount;
        dev.channels = 2;

        // open audio device
        if (SDL_OpenAudio(&dev, null)) {
            return false;
        };

        // setup mixer
        mixer.rate(rate);

        // start audio playback
        SDL_PauseAudio(0);

        // success
        return true;
    };
};
