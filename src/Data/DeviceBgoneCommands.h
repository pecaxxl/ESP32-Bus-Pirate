#pragma once

#include <vector>
#include "Enums/InfraredProtocolEnum.h"
#include "Models/InfraredCommand.h"

inline const std::vector<InfraredCommand> deviceBgoneCommands = {
    {NECX2, 7, 7, 2},
    {NECX2, 7, -1, 2},
    {SAMSUNG32, 7, 7, 2},
    {SAMSUNG20, 7, -1, 2},
    {SAMSUNG36, 7, 7, 2},
    {NECX2, 4, -1, 8},
    {_RC5, 0, 0, 12},
    {SONY20, 1, 0, 21},
    {SONY20, 2, 0, 21},
    {_NEC2, 4, 0, 8},
    {_RC5, 1, 0, 12},
    {_NEC, 4, 0, 8},
    {_RC6, 0, 0, 12},
    {_NEC, 25, 0, 24},
    {_NEC2, 2, 125, 70},
    {_NEC2, 4, -1, 8},
    {_NEC2, 64, -1, 18},
    {_NEC2, 80, -1, 63},
    {_NEC2, 210, 109, 4},
    {FUJITSU, 64, 8, 2},
    {AIWA, 110, 0, 0},
    {_NEC2, 80, -1, 63},
    {_RC5, 0, -1, 32},
    {_NEC2, 234, 199, 33},
    {_NEC, 73, 0, 26},
    {_NEC2, 100, 70, 93},
    {_NEC2, 0, 127, 10},
    {_NEC2, 128, 216, 47},
    {SONY20, 90, 27, 21},
    {_NEC2, 0, 127, 21},
    {_NEC2, 0, 191, 13},
    {_NEC2, 0, 127, 21},
    {_PANASONIC, 8, 0, 61},
    {_JVC, 0, 31, 218},
    {_DENON, 4, -1, 8},
    {_NEC2, 120, -1, 204},
    {_NEC2, 231, -1, 24},
    {_NEC2, 170, -1, 72},
    {MITSUBISHI, 71, -1, 2},
    {_SHARP, 1, -1, 233},
    {_NEC2, 32, -1, 65},
    {_NEC, 60, -1, 204},
    {_NEC2, 186, 160, 76},
    {_NEC, 56, 0, 18},
    {RC6_6_20, 0, 0, 12},
    {NEC1, 0, -1, 32},
    {SAMSUNG32, 0, -1, 44},
    {SONY12, 1, 0, 21},
    {JVC_48, 0, 0, 12},
    {_APPLE, 238, 0, 126},
    {LOGITECH, 0, 0, 12},
    {PANASONIC2, 1, 1, 30},
    {SHARP_2_, 1, 0, 169},
    {BOSE, 1, -1, 10},
    {DIRECTV, 0, 0, 15},
    {MCE, 0, 0, 1},
    {ZENITH, 1, 0, 7},
    {RCA, 0, -1, 12},
    {GRUNDIG16, 0, 0, 10},
    {TIVO_UNIT_0, 0, -1, 45},
    {STREAMZAP, 0, 0, 1},
    {PIONEER, 1, 0, 64},
    {AKAI, 0, -1, 14},
    {THOMSON, 0, 0, 6},
    {XMP, 0, -1, 22},
    {ZAPTOR_56, 0, 0, 5},
    {BARCO, 0, 0, 66},
    {BOSE, 0, -1, 71},
    {PIONEER, 0, -1, 1260},
    {AKAI, 0, -1, 9},
    {AIWA, 0, -1, 705},
    {PANASONIC2, 0, -1, 51},
    {BOSE, 0, -1, 71},
    {PANASONIC2, 0, -1, 9},
    {LOGITECH, 0, -1, 1},
    {BOSE, 0, -1, 11},
    {BOSE, 0, -1, 16},
    {BOSE, 0, -1, 17},
    {PIONEER, 0, -1, 12},
    {AIWA, 0, -1, 705},
    {AKAI, 0, -1, 9},
    {PANASONIC2, 0, -1, 51},
    {DIRECTV, 0, -1, 9},
    {ZENITH, 0, -1, 9},
    {RCA, 0, -1, 392},
    {GRUNDIG16, 0, -1, 398},
    {TIVO_UNIT_0, 0, -1, 45},
    {STREAMZAP, 0, -1, 1},
    {XMP, 0, -1, 22},
    {ZAPTOR_56, 0, -1, 5},
    {BARCO, 0, -1, 66},
    { _NEC, 0, -1, 57 },
    { _NEC, 0, -1, 135 }, 
    { F12, 0, -1, 128 },


    // NEC‑based universal power codes
    { _NEC2,        0,  -1, 16 },   // NEC generic power
    { _NEC2,       32,  -1, 65 },   // NEC power variant
    { _NEC2,      128, 216, 47 },   // NEC power (obsure)
    { _NEC2,       80,  -1, 63 },   // NEC power (alt)
    { _NEC2,      210, 109, 4  },   // NEC power (alt2)

    // Samsung power
    { SAMSUNG32,   0,  -1, 44 },   // Samsung 32bit power
    { SAMSUNG36,   0,  -1, 44 },   // Samsung 36bit variant

    // Sony
    { SONY20,      1,   0, 21 },   // Sony20 standard power
    { SONY12,      1,   0, 21 },   // Sony12 standard power

    // Panasonic / Kaseikyo
    { _PANASONIC,  8,   0, 61 },   // Panasonic power
    { PANASONIC2,  1,   1, 30 },   // Panasonic2 variant
    { PANASONIC2,  0,  -1, 51 },   // Panasonic2 alt

    // JVC
    { JVC_48,      0,   0, 12 },   // JVC basic power

    // Philips RC‑5 / RC‑6 standards
    { _RC5,        0,  -1, 32 },   // RC‑5 power (Philips)
    { RC6_6_20,    0,   0, 12 },   // RC‑6 power

    // RCA
    { RCA,         0,  -1, 12 },   // RCA power

    // Toshiba/Zenith
    { ZENITH,      1,   0, 7  },   // Zenith power

    // Bose
    { BOSE,        1,  -1, 10 },   // Bose power

    // Pioneer
    { PIONEER,     0,  -1, 1260 }, // Pioneer power

    // Directv, MCE
    { DIRECTV,     0,  -1, 9  },   // DirecTV power
    { MCE,         0,   0, 1  },   // MCE / Media Center power

    // Aiwa, Akai
    { AIWA,        0,  -1, 705 },  // Aiwa power<z
    { AKAI,        0,  -1, 9  },   // Akai power

    // Sharp
    { _SHARP,      1,  -1, 233 },  // Sharp power
    { SHARP_2_,    1,   0, 169 },  // Sharp2 power

    // One‑For‑All / Streamzap
    { STREAMZAP,   0,   0, 1  },   // StreamZap power
    { LOGITECH,    0,   0, 12 },   // Logitech / universal

    // Pioneer / Bose duplicates for universality
    { PIONEER,     1,   0, 64 },   // Pioneer alt
    { BOSE,        0,  -1, 71 }   // Bose alt
};