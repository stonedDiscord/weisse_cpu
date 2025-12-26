#!/usr/bin/env python3
"""
MIDI to Track Converter for 8085 Music Player

This script converts MIDI files to C arrays for the 8085 music player.
It extracts note events, timing, and generates the track.c file.

Usage:
    python3 parse.py <input.mid> <output.c>
    
Example:
    python3 parse.py apple.mid track.c

The output file contains an array with entries for:
- note: Note name (NOTE_C, NOTE_D, NOTE_E, etc.)
- octave: Octave (A8, A7, A6, A5)
- duration: Note duration (DURATION_EIGHTH, DURATION_QUARTER, etc.)
- delay: Delay in milliseconds between note starts

This array is used by the 8085 music player to play back the MIDI file.
"""

import mido
import sys

note_name_list = ['NOTE_INVALID', 'NOTE_C', 'NOTE_B', 'NOTE_AS', 'NOTE_A', 'NOTE_GS', 'NOTE_G', 'NOTE_FS', 'NOTE_F', 'NOTE_E', 'NOTE_DS', 'NOTE_D', 'NOTE_CS']
octave_name_list = ['A8', 'A7', 'A6', 'A5']
duration_name_list = ['DURATION_EIGHTH', 'DURATION_QUARTER', 'DURATION_HALF', 'DURATION_WHOLE']

def midi_note_to_enum(midi_note):
    """Convert MIDI note number to note enum value"""
    note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
    
    # Handle sharps and flats
    octave = (midi_note // 12) - 1
    note_in_octave = midi_note % 12
    
    # Map to our enum (flats are mapped to sharps in reverse order)
    note_map = {
        0: 1,   # C  -> NOTE_C
        1: 12,  # C# -> NOTE_CS
        2: 11,  # D  -> NOTE_D  
        3: 10,  # D# -> NOTE_DS
        4: 4,   # E  -> NOTE_E
        5: 8,   # F  -> NOTE_F
        6: 7,   # F# -> NOTE_FS
        7: 6,   # G  -> NOTE_G
        8: 5,   # G# -> NOTE_GS
        9: 4,   # A  -> NOTE_A (but will be overwritten below)
        10: 3,  # A# -> NOTE_AS
        11: 2,  # B  -> NOTE_B
    }
    
    # Special handling for A and A#
    if note_in_octave == 9:  # A
        return 4, octave  # NOTE_A
    elif note_in_octave == 10:  # A#
        return 3, octave  # NOTE_AS
    
    return note_map[note_in_octave], octave

def midi_note_to_enum_v2(midi_note):
    """Alternative mapping based on the existing track.c example"""
    note_in_octave = midi_note % 12
    octave = (midi_note // 12) - 1
    
    # Map MIDI note to our enum (using flat/sharp mappings from the existing code)
    if note_in_octave == 0:  # C
        note_enum = 1  # NOTE_C
    elif note_in_octave == 1:  # C#
        note_enum = 12  # NOTE_CS
    elif note_in_octave == 2:  # D
        note_enum = 11  # NOTE_D
    elif note_in_octave == 3:  # D#
        note_enum = 10  # NOTE_DS
    elif note_in_octave == 4:  # E
        note_enum = 9  # NOTE_E
    elif note_in_octave == 5:  # F
        note_enum = 8  # NOTE_F
    elif note_in_octave == 6:  # F#
        note_enum = 7  # NOTE_FS
    elif note_in_octave == 7:  # G
        note_enum = 6  # NOTE_G
    elif note_in_octave == 8:  # G#
        note_enum = 5  # NOTE_GS
    elif note_in_octave == 9:  # A
        note_enum = 4  # NOTE_A
    elif note_in_octave == 10:  # A#
        note_enum = 3  # NOTE_AS
    elif note_in_octave == 11:  # B
        note_enum = 2  # NOTE_B
    else:
        note_enum = 0  # NOTE_INVALID
    
    return note_enum, octave

def octave_to_enum(octave):
    """Convert octave number to octave enum value"""
    # Map octave numbers to our enum (shifted down since MIDI octaves start at -1)
    if octave >= 5:
        return 0  # A8
    elif octave >= 4:
        return 1  # A7
    elif octave >= 3:
        return 2  # A6
    else:
        return 3  # A5

def ticks_to_duration(ticks, ticks_per_beat, tempo):
    """Convert MIDI ticks to duration enum"""
    # Calculate beats from ticks
    beats = ticks / ticks_per_beat
    
    # Map to duration enum based on the example timing
    # DURATION_EIGHTH = 0, DURATION_QUARTER = 1, DURATION_HALF = 2, DURATION_WHOLE = 3
    if beats <= 0.5:
        return 0  # DURATION_EIGHTH
    elif beats <= 1.0:
        return 1  # DURATION_QUARTER
    elif beats <= 2.0:
        return 2  # DURATION_HALF
    else:
        return 3  # DURATION_WHOLE

def parse_midi_file(filename):
    """Parse MIDI file and extract note events"""
    try:
        mid = mido.MidiFile(filename)
    except Exception as e:
        print(f"Error opening MIDI file: {e}")
        return []
    
    # Get tempo (default to 500000 microseconds per beat if not specified)
    tempo = 500000
    for track in mid.tracks:
        for msg in track:
            if msg.type == 'set_tempo':
                tempo = msg.tempo
                break
    
    # Calculate ticks per quarter note
    ticks_per_beat = mid.ticks_per_beat
    
    # Process all tracks to extract note events
    note_events = []
    
    for track in mid.tracks:
        track_time = 0
        active_notes = {}  # note -> start_time
        
        for msg in track:
            track_time += msg.time
            
            if msg.type == 'note_on' and msg.velocity > 0:
                # Note start
                if msg.note not in active_notes:
                    active_notes[msg.note] = track_time
                    
            elif msg.type == 'note_off' or (msg.type == 'note_on' and msg.velocity == 0):
                # Note end
                if msg.note in active_notes:
                    start_time = active_notes[msg.note]
                    duration_ticks = track_time - start_time
                    
                    # Convert to our format
                    note_enum, midi_octave = midi_note_to_enum_v2(msg.note)
                    octave_enum = octave_to_enum(midi_octave)
                    duration_enum = ticks_to_duration(duration_ticks, ticks_per_beat, tempo)
                    
                    # Calculate delay in milliseconds (will convert to relative delays later)
                    ms_per_beat = tempo / 1000.0
                    absolute_delay_ms = int((start_time / ticks_per_beat) * ms_per_beat)
                    
                    # Skip NOTE_INVALID entries (they would be silent)
                    if note_enum == 0:  # NOTE_INVALID
                        del active_notes[msg.note]
                        continue
                    
                    note_events.append({
                        'note': note_enum,
                        'octave': octave_enum,
                        'duration': duration_enum,
                        'absolute_delay': absolute_delay_ms
                    })
                    
                    del active_notes[msg.note]
    
    # Convert absolute delays to relative delays between consecutive events
    for i in range(1, len(note_events)):
        delay = note_events[i]['absolute_delay'] - note_events[i-1]['absolute_delay']
        # Ensure no negative delays - use absolute value for overlapping notes
        note_events[i]['delay'] = abs(delay)
    
    # First note has delay of 0 (starts immediately)
    if note_events:
        note_events[0]['delay'] = 0
    
    # Remove the temporary absolute_delay field
    for event in note_events:
        if 'absolute_delay' in event:
            del event['absolute_delay']
    
    return note_events

def generate_track_c(note_events):
    """Generate C code for track array"""
    if not note_events:
        return "struct noteData track[] = {};\n"
    
    lines = ["struct noteData track[] = {"]
    
    for i, event in enumerate(note_events):
        # Map note enum values to string names
        note_names = ['INVALID', 'C', 'B', 'AS', 'A', 'GS', 'G', 'FS', 'F', 'E', 'DS', 'D', 'CS']
        line = f"    {{ .note = NOTE_{note_names[event['note']]}, " \
               f".octave = {['A8', 'A7', 'A6', 'A5'][event['octave']]}, " \
               f".duration = DURATION_{['EIGHTH', 'QUARTER', 'HALF', 'WHOLE'][event['duration']]}, " \
               f".length = {event['delay']} }}"
        
        if i < len(note_events) - 1:
            line += ","
        lines.append(line)
    
    lines.append("};")
    return "\n".join(lines) + "\n"

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 parse.py <input.mid> <output.c>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    print(f"Parsing MIDI file: {input_file}")
    note_events = parse_midi_file(input_file)
    
    if not note_events:
        print("No note events found in MIDI file")
        sys.exit(1)
    
    print(f"Found {len(note_events)} note events")
    
    # Generate C code
    c_code = generate_track_c(note_events)
    
    # Write to output file
    with open(output_file, 'w') as f:
        f.write(c_code)
    
    print(f"Generated track array written to: {output_file}")

if __name__ == "__main__":
    main()
