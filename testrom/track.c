#include "music.h"

struct noteData track[] = {
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //SA
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //KU
    { .note = NOTE_B, .octave = A7, .duration = DURATION_HALF,    .length = 200 }, //RA

    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //SA
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //KU
    { .note = NOTE_B, .octave = A7, .duration = DURATION_HALF,    .length = 200 }, //RA

    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //YA
    { .note = NOTE_B, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //YO
    { .note = NOTE_C, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //I
    { .note = NOTE_B, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //NO
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //SO
    { .note = NOTE_B, .octave = A7, .duration = DURATION_QUARTER, .length = 50 }, //RA
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 50 }, //-
    { .note = NOTE_F, .octave = A7, .duration = DURATION_HALF,    .length = 200 }, //WA

    { .note = NOTE_E, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //MI
    { .note = NOTE_D, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //WA
    { .note = NOTE_E, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //TA
    { .note = NOTE_F, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //SU
    { .note = NOTE_E, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //KA
    { .note = NOTE_E, .octave = A7, .duration = DURATION_QUARTER, .length = 50 }, //GI
    { .note = NOTE_C, .octave = A8, .duration = DURATION_QUARTER, .length = 50 }, //-
    { .note = NOTE_B, .octave = A8, .duration = DURATION_HALF,    .length = 200 }, //RI

    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //KA
    { .note = NOTE_B, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //SU
    { .note = NOTE_C, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //MI
    { .note = NOTE_B, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //KA
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //KU
    { .note = NOTE_B, .octave = A7, .duration = DURATION_QUARTER, .length = 50 }, //MO
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 50 }, //-
    { .note = NOTE_F, .octave = A7, .duration = DURATION_HALF,    .length = 200 }, //KA

    { .note = NOTE_E, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //NI
    { .note = NOTE_D, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //O
    { .note = NOTE_E, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //I
    { .note = NOTE_F, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //ZO
    { .note = NOTE_E, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //I
    { .note = NOTE_E, .octave = A7, .duration = DURATION_QUARTER, .length = 50 }, //ZU
    { .note = NOTE_C, .octave = A8, .duration = DURATION_QUARTER, .length = 50 }, //-
    { .note = NOTE_B, .octave = A8, .duration = DURATION_HALF,    .length = 200 }, //RU

    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //I
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //ZA
    { .note = NOTE_B, .octave = A7, .duration = DURATION_HALF,    .length = 200 }, //YA
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //I
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 100 }, //ZA
    { .note = NOTE_B, .octave = A7, .duration = DURATION_HALF,    .length = 200 }, //YA

    { .note = NOTE_E, .octave = A7, .duration = DURATION_QUARTER, .length = 200 }, //MI
    { .note = NOTE_F, .octave = A7, .duration = DURATION_QUARTER, .length = 200 }, //NI
    { .note = NOTE_B, .octave = A7, .duration = DURATION_QUARTER, .length = 150 }, //YU
    { .note = NOTE_A, .octave = A7, .duration = DURATION_QUARTER, .length = 150 }, //-
    { .note = NOTE_F, .octave = A7, .duration = DURATION_QUARTER, .length = 200 }, //KA
    { .note = NOTE_E, .octave = A7, .duration = DURATION_HALF,    .length = 200 }, //N
};