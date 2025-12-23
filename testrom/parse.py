#!/usr/bin/env python3

import mido

note_name_list = ['NOTE_CL', 'NOTE_C', 'NOTE_B', 'NOTE_AS', 'NOTE_A', 'NOTE_GS', 'NOTE_G', 'NOTE_FS', 'NOTE_F', 'NOTE_E', 'NOTE_DS', 'NOTE_D', 'NOTE_CS']
octave_name_list = ['A8', 'A7', 'A6', 'A5']
duration_name_list = ['DURATION_EIGHTH', 'DURATION_QUARTER', 'DURATION_HALF', 'DURATION_WHOLE']

note_map = {0:1, 1:12, 2:11, 3:10, 4:9, 5:8, 6:7, 7:6, 8:5, 9:4, 10:3, 11:2}
octave_map_midi = {5:3, 6:2, 7:1, 8:0}

notes = []
active_notes = {}
current_time = 0

mid = mido.MidiFile('apple.mid')
for msg in mid:
    current_time += msg.time
    if msg.type == 'note_on' and msg.velocity > 0:
        active_notes[msg.note] = current_time
    elif msg.type == 'note_off' or (msg.type == 'note_on' and msg.velocity == 0):
        if msg.note in active_notes:
            start_time = active_notes.pop(msg.note)
            duration_ticks = current_time - start_time
            midi_note = msg.note
            note_index = midi_note % 12
            note = note_map[note_index]
            midi_octave = midi_note // 12
            octave = octave_map_midi.get(midi_octave, 1)
            beats = duration_ticks / mid.ticks_per_beat
            if beats <= 0.5:
                dur = 0
            elif beats <= 1:
                dur = 1
            elif beats <= 2:
                dur = 2
            else:
                dur = 3
            note_str = note_name_list[note]
            oct_str = octave_name_list[octave]
            dur_str = duration_name_list[dur]
            notes.append((note_str, oct_str, dur_str))

print("struct noteData track[] = {")
for note, oct, dur in notes:
    print(f"    {{{note}, {oct}, {dur}}},")
print("};")
