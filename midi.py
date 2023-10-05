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

    num_midi_channels = 16
    num_opl_channels = 8

    #

    opl_channel_off_timers = [1 for i in range(num_opl_channels)]
    opl_programs = [-1 for i in range(num_opl_channels)]
    opl_notes = [-1 for i in range(num_opl_channels)]
    opl_midi_channels = [-1 for i in range(num_opl_channels)]

    midi_programs = [-1 for i in range(num_midi_channels)]

    #

    def find_free_opl_channel(program):
        free_channel = -1
        highest_timer_value = 0
        for i, timer in enumerate(opl_channel_off_timers):
            if timer > highest_timer_value:
                if opl_programs[i] == program:
                    highest_timer_value = timer
                    free_channel = i

        if free_channel != -1:
            return free_channel

        free_channel = -1
        highest_timer_value = 0
        for i, timer in enumerate(opl_channel_off_timers):
            if timer > highest_timer_value:
                highest_timer_value = timer
                free_channel = i

        return free_channel

    #

    channel_volumes = [90 for i in range(num_midi_channels)] # 90 should be 0dB, 127 would be +6dB
    channel_expressions = [127 for i in range(num_midi_channels)] 

    #

    time_divider = 4

    num_note_ons = 0
    num_note_offs = 0
    num_program_changes = 0
    num_cc_changes = 0

    for event in midifile:
        #delta_ticks = event.delta_miditicks
        delta_us_accumulator += event.delta_us

        for i, timer in enumerate(opl_channel_off_timers):
            if timer > 0:
                opl_channel_off_timers[i] += event.delta_us

        if hasattr(event, "channel") and event.channel != 10:
            time_delta = delta_us_accumulator // time_divider
            if time_delta >= 65536:
                print("Too long event time delta")
                abort()

            if event.status == umidiparser.NOTE_ON and event.velocity > 0:
                opl_channel = find_free_opl_channel(midi_programs[event.channel])
                if opl_channel == -1:
                    print("No free OPL channels")
                    continue

                if event.note < 12:
                    print("Warning: Low note")

                note = event.note - 12
                program = midi_programs[event.channel]
                adjusted_velocity = adjust_velocity(event.velocity, channel_volumes[event.channel], channel_expressions[event.channel])

                opl_channel_off_timers[opl_channel] = 0
                opl_notes[opl_channel] = note
                opl_midi_channels[opl_channel] = event.channel

                if opl_programs[opl_channel] != program:
                    opl_programs[opl_channel] = program
                    f.write("{ " + str(time_delta) + ", " + str(opl_channel) + ", " + str(program) + ", 255 },\n")
                    f.write("{ 0, " + str(opl_channel) + ", " + str(note) + ", " + str(adjusted_velocity) + " },\n")
                    num_program_changes += 1
                else:
                    f.write("{ " + str(time_delta) + ", " + str(opl_channel) + ", " + str(note) + ", " + str(adjusted_velocity) + " },\n")

                delta_us_accumulator -= time_delta * time_divider
                num_note_ons += 1

            elif event.status == umidiparser.NOTE_OFF or (event.status == umidiparser.NOTE_ON and event.velocity == 0):

                if event.note < 12:
                    print("Warning: Low note")

                note = event.note - 12

                opl_channel = -1
                for i in range(num_opl_channels):
                    if opl_midi_channels[i] == event.channel and opl_notes[i] == note and opl_channel_off_timers[i] == 0:
                        opl_channel = i
                        break

                if opl_channel == -1:
                    continue

                opl_channel_off_timers[opl_channel] = 1

                f.write("{ " + str(time_delta) + ", " + str(opl_channel) + ", 0, 0 },\n")

                delta_us_accumulator -= time_delta * time_divider
                num_note_offs += 1

            elif event.status == umidiparser.PROGRAM_CHANGE:
                #f.write("{ " + str(time_delta) + ", " + str(event.channel) + ", " + str(event.program) + ", 255 },\n")
                midi_programs[event.channel] = event.program

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
