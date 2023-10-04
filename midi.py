#!/usr/bin/env python3

import umidiparser

name = "song"

midifile = umidiparser.MidiFile("aim.mid")

print("Ticks per quarter: " + str(midifile.miditicks_per_quarter))

def adjust_velocity(v, volume, expression):
    r = int(v * volume / 127 * expression / 127 + 0.5)
    if r > 127:
        r = 127
    return r

with open(name + ".h", "w") as f:
    f.write("typedef struct song_event_t {\n")
    f.write("    uint16_t time_delta;\n")
    f.write("    uint8_t channel;\n")
    f.write("    uint8_t note;\n")
    f.write("    uint8_t velocity;\n")
    f.write("} song_event_t;\n\n")

    f.write("const song_event_t song_events[] = {\n")

    delta_us_accumulator = 0

    num_channels = 16

    channel_volumes = [90 for i in range(num_channels)] # 90 should be 0dB, 127 would be +6dB
    channel_expressions = [127 for i in range(num_channels)] 

    time_divider = 32

    num_note_ons = 0
    num_note_offs = 0
    num_program_changes = 0
    num_cc_changes = 0

    for event in midifile:
        #delta_ticks = event.delta_miditicks
        delta_us_accumulator += event.delta_us
        if hasattr(event, "channel") and event.channel != 10:
            time_delta = delta_us_accumulator // time_divider
            if time_delta >= 65536:
                print("Too long event time delta")
                abort()

            if event.status == umidiparser.NOTE_ON and event.velocity > 0:
                if event.note < 12:
                    print("Warning: Low note")
                note = event.note - 12
                adjusted_velocity = adjust_velocity(event.velocity, channel_volumes[event.channel], channel_expressions[event.channel])
                f.write("{ " + str(time_delta) + ", " + str(event.channel) + ", " + str(note) + ", " + str(adjusted_velocity) + " },\n")
                delta_us_accumulator -= time_delta * time_divider
                num_note_ons += 1

            elif event.status == umidiparser.NOTE_OFF or (event.status == umidiparser.NOTE_ON and event.velocity == 0):
                if event.note < 12:
                    print("Warning: Low note")
                note = event.note - 12
                f.write("{ " + str(time_delta) + ", " + str(event.channel) + ", " + str(note) + ", 0 },\n")
                delta_us_accumulator -= time_delta * time_divider
                num_note_offs += 1

            elif event.status == umidiparser.PROGRAM_CHANGE:
                f.write("{ " + str(time_delta) + ", " + str(event.channel) + ", " + str(event.program) + ", 255 },\n")
                delta_us_accumulator -= time_delta * time_divider
                num_program_changes += 1

            elif event.status == umidiparser.CONTROL_CHANGE:
                #print("Channel " + str(event.channel) + " control " + str(event.control) + " value " + str(event.value))
                if event.control == 7:
                    channel_volumes[event.channel] = event.value
                    num_cc_changes += 1
                elif event.control == 11:
                    channel_expressions[event.channel] = event.value
                    num_cc_changes += 1

    f.write("{ 65535, 0, 0, 0 }\n")
    f.write("};\n")

    print("Num note ons: " + str(num_note_ons))
    print("Num note offs: " + str(num_note_offs))
    print("Num program changes: " + str(num_program_changes))
    print("Num control changes: " + str(num_cc_changes))
