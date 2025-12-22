#!/usr/bin/env python3

note_names = {
    'C': 'NOTE_C',
    'c': 'NOTE_B',  # C flat
    'D': 'NOTE_D',
    'd': 'NOTE_CS', # D flat
    'E': 'NOTE_E',
    'e': 'NOTE_DS', # E flat
    'F': 'NOTE_F',
    'f': 'NOTE_E',  # F flat
    'G': 'NOTE_G',
    'g': 'NOTE_FS', # G flat
    'A': 'NOTE_A',
    'a': 'NOTE_GS', # A flat
    'B': 'NOTE_B',
    'b': 'NOTE_AS', # B flat
}

octave_names = {
    0: 'A8',
    1: 'A7',
    2: 'A6',
    3: 'A5',
}

duration_names = {
    0: 'DURATION_EIGHTH',
    1: 'DURATION_QUARTER',
    2: 'DURATION_HALF',
    3: 'DURATION_WHOLE',
}

octave_map = {
    '3': 0,
    '4': 1,
    '5': 2,
    '6': 3,
}

def duration_map(d):
    if d == 1: return 0  # EIGHTH
    elif d == 2: return 1  # QUARTER
    elif d == 3: return 2  # HALF
    else: return 3  # WHOLE

notes = []

with open('music.txt', 'r') as f:
    lines = f.readlines()

i = 0
while i < len(lines):
    line = lines[i].strip()
    if line and line[0].isdigit():
        octave_str = line[0]
        octave = octave_map.get(octave_str, 1)  # default 1
        parts = line.split('|')
        if len(parts) >= 3:
            s = parts[1]
            j = 0
            while j < len(s):
                if s[j] == '-':
                    count = 0
                    while j < len(s) and s[j] == '-':
                        count += 1
                        j += 1
                    if count > 0:
                        notes.append(('NOTE_CL', octave_names[octave], duration_names[duration_map(count)]))
                elif s[j].isalpha():
                    note_char = s[j]
                    j += 1
                    count = 0
                    while j < len(s) and s[j] == '-':
                        count += 1
                        j += 1
                    duration = count + 1
                    note_str = note_names.get(note_char, 'NOTE_CL')
                    notes.append((note_str, octave_names[octave], duration_names[duration_map(duration)]))
                else:
                    j += 1
        i += 2  # skip the number line
    else:
        i += 1

print("struct noteData track[] = {")
for note, oct, dur in notes:
    print(f"    {{{note}, {oct}, {dur}}},")
print("};")
