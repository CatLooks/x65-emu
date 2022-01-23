// -- audio -- //

// constants
const int sampleRate = 44100;
const int sampleCount = 735;
const int sampleCost = 64;
const int waveSize = 512;

// waveform buffer
Uint8 waveBuffer[32768];

// channel object
class Channel {
    public:
    // constructor
    Channel () {
        m_active = false;
        m_phase = 0.0;
        m_freq = 0.0;
        m_lvol = 0.0;
        m_rvol = 0.0;
        m_jump = 0.0;
        m_rep = false;
    };
    // set frequency
    void freq(int value) {
        m_freq = value;
        m_phase = 0.0;
    };
    void freqL(Uint8 value) {
        m_freq = (m_freq & 0xFF00) | value;
    };
    void freqH(Uint8 value) {
        m_freq = (m_freq & 0x00FF) | ((value & 0x3F) << 8);
    };
    // set volume
    void volL(Uint8 value) {
        m_lvol = float(value) / 255.0;
    };
    void volR(Uint8 value) {
        m_rvol = float(value) / 255.0;
    };
    // set loop
    void loopL(Uint8 value) {
        m_loop = (m_loop & 0xFF00) | value;
        m_jump = float(m_loop) / 512;
    };
    void loopH(Uint8 value) {
        if (value >= 2) {
            m_rep = false;
        } else {
            m_loop = (m_loop & 0x00FF) | (value << 8);
            m_jump = float(m_loop) / 512;
            m_rep = true;
        };
    };
    // set wave form
    void wave(Uint8 id) {
        m_wave = id & 0x7F;
        m_phase = 0.0;
    };
    // toggle state
    void enable(bool state) {
        m_active = state;
        m_phase = 0.0;
    };
    // get next sample
    Uint32 next(unsigned int rate) {
        // check for state
        if (!m_active)
            return 0;

        // update phase
        m_phase += float(m_freq) / rate;
        if (m_phase >= 1.0) {
            if (m_rep) {
                m_phase += m_jump;
                m_phase = fmod(m_phase, 1.0);
            } else return 0;
        };

        // return samples
        Uint16 full = waveBuffer[int(m_phase * waveSize) + waveSize * m_wave] * sampleCost;
        return Uint16(full * m_lvol) << 16 | Uint16(full * m_rvol);
    };

    private:
    bool m_active;
    float m_phase;
    float m_lvol;
    float m_rvol;
    float m_jump;
    short m_freq;
    bool m_rep;
    bt m_wave;
    wt m_loop;
};

// mixer object
class Mixer {
    public:
    // set sample rate
    void rate(unsigned int value) {
        m_rate = value;
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
