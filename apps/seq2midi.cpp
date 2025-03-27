// seq2midi.cpp -- simple sequence player, intended to help test/demo
// the allegro code

#include <algorithm>
#include <cmath>
#include <cstring>
#include "allegro.h"
#include "porttime.h"
#include "portmidi.h"
#include "midicode.h"

using namespace std;


double time_elapsed()
{
    return Pt_Time() * 0.001;
}


void wait_until(double time)
{
    // print "." to stdout while waiting
    static double last_time = 0.0;
    double now = time_elapsed();
    last_time = std::min(now, last_time);
    while (now < time) {
        Pt_Sleep(1);
        now = time_elapsed();
        long now_sec = (long) now;
        long last_sec = (long) last_time;
        if (now_sec > last_sec) {
            fprintf(stdout, ".");
            fflush(stdout);
            last_time = now;
        }
    }
}


#define never 1000000 // represents infinite time

void midi_note_on(PortMidiStream *midi, double when, int chan, int key, int loud)
{
    unsigned long timestamp = (unsigned long) (when * 1000);
    chan = chan & 15;
    key = std::clamp(key, 0, 127);
    loud = std::clamp(loud, 0, 127);
    unsigned long data = Pm_Message(0x90 + chan, key, loud);
    Pm_WriteShort(midi, timestamp, data);
}


static void midi_channel_message_2(PortMidiStream *midi, double when, 
                                   int status, int chan, int data)
{
    unsigned long timestamp = (unsigned long) (when * 1000);
    chan = chan & 15;
    data = std::clamp(data, 0, 127);
    unsigned long msg = Pm_Message(status + chan, data, 0);
    Pm_WriteShort(midi, timestamp, msg);
}


static void midi_channel_message(PortMidiStream *midi, double when, 
                                 int status, int chan, int data, int data2)
{
    unsigned long timestamp = (unsigned long) (when * 1000);
    chan = chan & 15;
    data = std::clamp(data, 0, 127);
    data2 = std::clamp(data, 0, 127);
    unsigned long msg = Pm_Message(status + chan, data, data2);
    Pm_WriteShort(midi, timestamp, msg);
}


static const char *pressure_attr;
static const char *bend_attr;
static const char *program_attr;


void send_midi_update(Alg_update_ptr u, PortMidiStream *midi)
{
    if (u->get_attribute() == pressure_attr) {
        if (u->get_identifier() < 0) {
            midi_channel_message_2(midi, u->time, MIDI_TOUCH, u->chan,
                                   (int) (u->get_real_value() * 127));
        } else {
            midi_channel_message(midi, u->time, MIDI_POLY_TOUCH, u->chan, 
                                 u->get_identifier(), 
                                 (int) (u->get_real_value() * 127));
        }
    } else if (u->get_attribute() == bend_attr) {
        int bend = std::lround((u->get_real_value() + 1) * 8192);
        bend = std::clamp(bend, 0, 8191);
        midi_channel_message(midi, u->time, MIDI_BEND, u->chan, 
                             bend >> 7, bend & 0x7F);
    } else if (u->get_attribute() == program_attr) {
        midi_channel_message_2(midi, u->time, MIDI_CH_PROGRAM, 
                               u->chan, u->get_integer_value());
    } else if (strncmp("control", u->get_attribute(), 7) == 0 &&
               u->get_update_type() == 'r') {
        int control = atoi(u->get_attribute() + 7);
        int val = std::lround(u->get_real_value() * 127);
        midi_channel_message(midi, u->time, MIDI_CTRL, u->chan, control, val);
    }
}


void seq2midi(Alg_seq &seq, PortMidiStream *midi)
{
    // prepare by doing lookup of important symbols
    pressure_attr = symbol_table.insert_string("pressurer") + 1;
    bend_attr = symbol_table.insert_string("bendr") + 1;
    program_attr = symbol_table.insert_string("programi") + 1;

    Alg_iterator iterator(&seq, true);
    iterator.begin();
    bool note_on;
    Alg_event_ptr e = iterator.next(&note_on);
    Pt_Start(1, nullptr, nullptr); // initialize time
    while (e) {
        double next_time = (note_on ? e->time : e->get_end_time());
        wait_until(next_time);
        if (e->is_note() && note_on) { // process notes here
            // printf("Note at %g: chan %d key %d loud %d\n",
            //        next_time, e->chan, e->key, (int) e->loud);
            midi_note_on(midi, next_time, e->chan, e->get_identifier(),
                         (int) e->get_loud());
        } else if (e->is_note()) { // must be a note off
            midi_note_on(midi, next_time, e->chan, e->get_identifier(), 0);
        } else if (e->is_update()) { // process updates here
            Alg_update_ptr u = (Alg_update_ptr) e; // coerce to proper type
            send_midi_update(u, midi);
        } 
        // add next note
        e = iterator.next(&note_on);
    }
    iterator.end();
}


void seq_play(Alg_seq &seq)
{
    PortMidiStream *mo;
    Pm_Initialize();
    PmDeviceID dev = Pm_GetDefaultOutputDeviceID();
    // note that the Pt_Time type cast is required because Pt_Time does 
    // not take an input parameter, whereas for generality, PortMidi
    // passes in a void * so the time function can get some context.
    // It is safe to call Pt_Time with a parameter -- it will just be ignored.
    if (Pm_OpenOutput(&mo, dev, nullptr, 256, 
                      (PmTimestamp (*)(void *))&Pt_Time, nullptr, 100) == pmNoError) {
        seq2midi(seq, mo);
        wait_until(time_elapsed() + 1);
        Pm_Close(mo);
    }
    Pm_Terminate();
    return;
}
