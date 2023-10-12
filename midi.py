#!/usr/bin/env python3

import umidiparser

name = "song"

#midifile = umidiparser.MidiFile("/Users/aarni/Library/CloudStorage/Dropbox/Test MIDI/priv45.mid")
midifile = umidiparser.MidiFile("/Users/aarni/Library/CloudStorage/Dropbox/COM Jam 2023/MIDI/song.mid")

print("Ticks per quarter: " + str(midifile.miditicks_per_quarter))

num_note_ons = 0
num_note_offs = 0
num_cc_changes = 0
num_delta_times = 0

velocity_multiplier = 1.9

def adjust_velocity(v, volume, expression):
    v *= velocity_multiplier
    r = int(v * volume / 127 * expression / 127 + 0.5)
    if r > 127:
        r = 127
        print("Clamped velocity: " + str(v));
    return r

def write_value(f, v, writing_index):
    if writing_index != 0:
        if (writing_index & 15) == 15:
            f.write(",\n")
        else:
            f.write(", ")

    f.write(str(v))

    return writing_index + 1

def write_delta_time(f, t, writing_index):
    a = t >> 16
    if a >= 0x7f: # 255 is reserved for termination
        print("Too long delta time")
        exit()

    b = (t >> 8) & 0xff
    c = t & 0xff

    if a == 0 and b < 0x40:
        writing_index = write_value(f, 0x40 | b, writing_index)
        writing_index = write_value(f, c, writing_index)
    else:
        writing_index = write_value(f, 0x80 | a, writing_index)
        writing_index = write_value(f, b, writing_index)
        writing_index = write_value(f, c, writing_index)

    return writing_index

with open(name + ".h", "w") as f:
    f.write("static const uint8_t song_events[] = {\n")

    writing_index = 0

    delta_us_accumulator = 0

    num_midi_channels = 16
    num_opl_channels = 8

    #

    opl_channel_off_timers = [1 for i in range(num_opl_channels)]
    opl_programs = [-1 for i in range(num_opl_channels)]
    opl_notes = [-1 for i in range(num_opl_channels)]
    opl_midi_channels = [-1 for i in range(num_opl_channels)]

    midi_programs = [0 for i in range(num_midi_channels)]

    #

    different_program_time_penalty = 1000000

    def find_free_opl_channel(program):
        free_channel = -1
        highest_timer_value = -10000000
        for i, timer in enumerate(opl_channel_off_timers):
            if timer > 0:
                if opl_programs[i] == program or opl_programs[i] == -1:
                    if timer - different_program_time_penalty > highest_timer_value:
                        highest_timer_value = timer - different_program_time_penalty
                        free_channel = i
                else:
                    if timer > highest_timer_value:
                        highest_timer_value = timer
                        free_channel = i

        if free_channel == -1:
            for i, timer in enumerate(opl_channel_off_timers):
                if opl_programs[i] == program or opl_programs[i] == -1:
                    return i

        return free_channel

    #

    channel_volumes = [127 for i in range(num_midi_channels)]
    channel_expressions = [127 for i in range(num_midi_channels)] 

    #

    delta_time_divider = 8
    delta_time_threshold = 2000

    for event in midifile:
        #delta_ticks = event.delta_miditicks
        delta_us_accumulator += event.delta_us

        for i, timer in enumerate(opl_channel_off_timers):
            if timer > 0:
                opl_channel_off_timers[i] += event.delta_us

        if hasattr(event, "channel") and event.channel != 10:
            if event.status == umidiparser.NOTE_ON and event.velocity > 0:
                opl_channel = find_free_opl_channel(midi_programs[event.channel])
                if opl_channel == -1:
                    print("No free OPL channels")
                    continue

                note = event.note
                if note >= 12:
                    note -= 12
                else:
                    print("Warning: Low note")

                program = midi_programs[event.channel]
                adjusted_velocity = adjust_velocity(event.velocity, channel_volumes[event.channel], channel_expressions[event.channel])

                opl_channel_off_timers[opl_channel] = 0
                opl_notes[opl_channel] = note
                opl_midi_channels[opl_channel] = event.channel

                if delta_us_accumulator >= delta_time_threshold:
                    delta_time = delta_us_accumulator // delta_time_divider
                    writing_index = write_delta_time(f, delta_time, writing_index)
                    delta_us_accumulator -= delta_time * delta_time_divider
                    num_delta_times += 1

                if opl_programs[opl_channel] != program:
                    opl_programs[opl_channel] = program
                    writing_index = write_value(f, 0x20 | opl_channel, writing_index)
                    writing_index = write_value(f, program, writing_index)

                writing_index = write_value(f, opl_channel, writing_index)
                writing_index = write_value(f, note, writing_index)
                writing_index = write_value(f, adjusted_velocity, writing_index)

                num_note_ons += 1

            elif event.status == umidiparser.NOTE_OFF or (event.status == umidiparser.NOTE_ON and event.velocity == 0):

                note = event.note
                if note >= 12:
                    note -= 12
                else:
                    print("Warning: Low note")

                opl_channel = -1
                for i in range(num_opl_channels):
                    if opl_midi_channels[i] == event.channel and opl_notes[i] == note and opl_channel_off_timers[i] == 0:
                        opl_channel = i
                        break

                if opl_channel == -1:
                    continue

                opl_channel_off_timers[opl_channel] = 1

                if delta_us_accumulator >= delta_time_threshold:
                    delta_time = delta_us_accumulator // delta_time_divider
                    writing_index = write_delta_time(f, delta_time, writing_index)
                    delta_us_accumulator -= delta_time * delta_time_divider
                    num_delta_times += 1

                writing_index = write_value(f, 0x10 + opl_channel, writing_index)

                num_note_offs += 1

            elif event.status == umidiparser.PROGRAM_CHANGE:
                print("Channel " + str(event.channel) + " program " + str(event.program))
                midi_programs[event.channel] = event.program

            elif event.status == umidiparser.CONTROL_CHANGE:
                #print("Channel " + str(event.channel) + " control " + str(event.control) + " value " + str(event.value))
                if event.control == 7:
                    channel_volumes[event.channel] = event.value * 127 / 100
                    if channel_volumes[event.channel] > 127:
                        channel_volumes[event.channel] = 127
                    num_cc_changes += 1
                elif event.control == 11:
                    channel_expressions[event.channel] = event.value
                    num_cc_changes += 1

    writing_index = write_value(f, 0xff, writing_index)
    f.write("\n};\n")

    print("Num bytes: " + str(writing_index))

print("Num note ons: " + str(num_note_ons))
print("Num note offs: " + str(num_note_offs))
print("Num control changes: " + str(num_cc_changes))
print("Num delta times: " + str(num_delta_times))
