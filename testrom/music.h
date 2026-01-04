#include <stdint.h>

typedef enum note {
    NOTE_INVALID = 0,
    NOTE_C = 1,
    NOTE_B = 2,
    NOTE_AS = 3,
    NOTE_A = 4,
    NOTE_GS = 5,
    NOTE_G = 6,
    NOTE_FS = 7,
    NOTE_F = 8,
    NOTE_E = 9,
    NOTE_DS = 10,
    NOTE_D = 11,
    NOTE_CS = 12,
} note_t;

typedef enum octave {
    A8 = 0,
    A7 = 1,
    A6 = 2,
    A5 = 3,
} octave_t;

typedef enum duration {
    DURATION_EIGHTH = 0,
    DURATION_QUARTER = 1,
    DURATION_HALF = 2,
    DURATION_WHOLE = 3,
} duration_t;

struct noteData
{
    note_t note: 4;
    octave_t octave: 2;
    duration_t duration: 2;
    uint8_t length; //divided by 8
};