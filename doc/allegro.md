# Allegro Music Representation Language

Allegro is a simple, declarative, text-based language for music representation. Allegro is very similar to Adagio, but it is extended somewhat to make the representation of beats and attributes simpler and more uniform. Allegro is also intended to represent Standard MIDI files, and there are standard attribute names and encodings in Allegro for metadata found in Standard MIDI files.

In addition to the language, there is an internal, object representation of Allegro, which is implemented in both C++ and [Serpent](https://www.cs.cmu.edu/~music/cmp/serpent/doc/serpent.htm) (a Python-like scripting language). Allegro files can be read into or written from Audacity. In particular, Audacity can convert binary Standard MIDI files into more human-readable Allegro files.

## Basic Concepts

Allegro represents notes, attribute/value pairs (typically updates to notes), and timing information in the form of tempo and beats. Allegro is a free-form ASCII text language similar to Adagio (a notation introduced in the CMU MIDI Toolkit). In its simplest usage, every line of text represents a note. Every note is a set of attributes separated by white space and ending with a newline. For example, the following indicates a quarter note at middle C:

```
Q C4
```

Allegro allows each note to be tagged so that the note can be referenced by updates at a later time.

## Standard Attributes and MIDI Data

Notes have a set of fields that correspond more-or-less to MIDI data.
The fields are:

- `key`: The key number or tag for this note. In MIDI, the key indicates the pitch, but in Allegro, the key can be any number. For example, key can be a sequence number so every note can have a unique name.
- `chan`: The MIDI channel. In Allegro, the channel can be any integer. This creates a two-dimentional name space for notes: &lt;chan, key&gt;
- `time`: The starting time of the note. This can be in beats or seconds. The Allegro data structure can convert from seconds to beats and back.
- `pitch`: The pitch of a note, expressed in the same units as MIDI, but this is a float rather than an integer so that precise tuning can be expressed. Note that in MIDI, pitch is implied by the key, but in Allegro, the key is just a name or a way to refer to a particular sound. The pitch can be specified separately. (If you are working with MIDI, you will probably want to specify pitch symbolically, e.g. C4, or as an integer, e.g. P60. The key number will default to the pitch.)
- `loud`: A parameter corresponding to MIDI velocity, but this is a float rather than an integer.

In addition, notes can have attribute/value pairs. In Allegro, the last letter of the attribute conveys the type of its value. For example, `bendr` should be read as attribute "bend" of type "r"eal. An "r" suffix is for "r"eal values (double precision floats). Some standard attributes are listed below:

| **MIDI Message Type** | **Allegro Attribute** |
|-----------------------|-----------------------|
| pressure (polyphonic aftertouch) | `pressurer` (`keyi` set to key number) |
| control change | `control0r`, `control1r`, `control2r`, ... `control127r` (note: values are normalized to the range [0..1]) |
| program change | `programi` |
| channel pressure (aftertouch) | `pressurer` (`keyi` is -1) |
| pitchbend | `bendr` (normalized to the range [-1..1]) |
| key signature | `keysigi` (number of sharps, negative of number of flats) and `modea` ('majora' for major, 'minora' for minor) |
| time signature | `timesig_numr` (the numerator) and `timesig_denr` (the denominator) |
| system exclusive | `sysexs` (message encoded as hex string) |

Standard MIDI File metadata is also encoded into allegro attributes as shown below:

| **Metadata Type** | **Allegro Attribute** |
|-------------------|-----------------------|
| text (`0x01`) | `texts` (note that the "`s`" suffix indicates a string value) |
| text (`0x02`) | `copyrights` |
| text (`0x04`) | `instruments` |
| text (`0x05`) | `lyrics` |
| text (`0x06`) | `markers` |
| text (`0x07`) | `cues` |
| text (`0x08`) | `miscs` |
| text (`0x54`) | `smpteoffsets` (SMPTE offset is encoded in a string as follows: `"ddfps:ddh:ddm:dds:dd.ddf"`, where `dd` is a two-digit decimal number. The one exception is that 30fps drop frame times are designated by `"29.97fps:..."`. Examples: `"24fps:00h:00m:00s:00.00f"`, `"29.97fps:00h:01m:40s:00.01f"`) |
| text (`0x20`) | MIDI Channel Prefix is used to assign a channel to a meta-event. In Allegro, this channel becomes a key specification, e.g. `K4 -cues:"cue15"`. |
| text (`0x7f`) | `sqspecifics` (data encoded as a hex string) |
| text (`0x03`) | `seqnames` (in Allegro files, sequence name is written after `#track 0`, but internally data is stored as if one had written, e.g. `-seqnames:"Jordu"`) |
| text (`0x03`) | `tracknames` (in Allegro files, track name is written after `#track n` where n > 0, but internally data is stored as if one had written, e.g. `-tracknames:"Bass"`) |

### MIDI Track Data

A few attributes, such as beatr and tempor, key signature information, and time signature information apply to all channels. The chani attribute can be set to -1 to indicate all channels.

Tracks from Standard MIDI Files are represented in the internal Allegro data structure. By default, all data goes to track zero (0), but a new track can be designated using the syntax "#track n", where n is an integer track number. All notes and updates following this track specification (until the next one) go into the designated track. From the programming API, track numbers can be encoded using the channel; for example, if the Allegro channel is:

```
track_number * 100 + midi_channel
```

then channel 1207 (decimal) will indicate track 12, midi channel 7. To preserve track association, the Allegro memory structures store each track separately. Per-track metadata is stored with the associated track with a channel value of -1.

## Allegro Syntax

Because Allegro has no nested expressions or complicated syntax, I will not present a detailed formal grammar. The previous section outlined the semantics of attributes. This section describes the syntax to denote an attribute. Additional syntax and semantics for tempo and beats is given in the next section.

Allegro notes and updates are denoted by a line of text containing a set of attributes. In addition, if a line begins with the character `#`, the line is not interpreted as a note or update. If the line begins with `# ` (hash, space) the line is a comment. The characters "#track" denote the beginning of a new track. The "new track" syntax is:

```
#track track_number "track_name"
```

where `track_number` is a decimal integer and `track_name` is any string of characters. Other metacommands beginning with `#` may be introduced in the future.

All strings are quoted with double (&quot;) quotes. To enclose quotes and other special characters in strings, use backslash (&quot;\\&quot;) as the escape character. You must type two backslashes to enter one backslash in a string, e.g. the string &quot;\\\\&quot; has a length of one.

Allegro is case-insensitive, that is, upper and lower case letters are treated as the same, except that full attribute names, when spelled out, are case-sensitive. By convention, attributes are lower case for uniformity and simplicity. In the following, `n` indicates a decimal integer, e.g. "7", and `r` indicates a decimal floating point number, e.g. "7.34".

Allegro events always carry a channel number, but some data is not channel-specific. These events use channel number -1. Similarly, some updates apply to every note on a channel and are therefore not key-specific. These updates use key number -1. Purely for aesthetic reasons, there is a special syntax for -1, namely, channel -1 can be indicated `V-` and key -1 can be indicated `K-`. (In common cases, "K-" can be omitted.)

| **Allegro Field or Attribute** | **Allegro Syntax** |
|--------------------------------|--------------------|
| `chan` | `Vn` |
| `key` | `Kn`, `Afn`, `An`, `Asn`, ..., `Gfn`, `Gn`, `Gsn` |
| `loud` | `Lr`, `Lppp`, ..., `Lfff` |
| `pitch` | `Pr`, `Afn`, `An`, `Asn`, ..., `Gfn`, `Gn`, `Gsn` |
| `dur` | `Ur`, `S`, `I`, `Q`, `H`, `W`, etc. |
| `time` | `Tr`, `TS`, ..., `TW`, etc. |
| (next time is not an stored attribute) | `Nr`, `NS`, ..., `NW`, etc. |
| (syntax for additional attributes/value pairs) | `-attribute:value` |

### Notes vs. Updates

A new note or sound object is created by the appearance of a field with any of the following initial letters: `P`, `A`, `B`, `C`, `D`, `E`, `F`, `G` (pitches), or `U` (duration). If neither an explicit pitch or duration appears, then an update is generated. Notes are considered to be objects, while update are considered to be messages that update or modify an existing objects. Thus, updates always refer to some existing object. In MIDI, the model is somewhat ambiguous, but typically a channel or (channel, key_number) pair is considered to be an object, and an update such as aftertouch would refer to one of these objects, as opposed to creating a new note.

### Sticky Attributes

Most attribute values need not be specified if they do not change from their lexically previous value. However, a blank line does not mean "repeat the last note"; at least one attribute must be present to signify anything at all. Only if a note is specified using at least a pitch or duration will certain default values be used. For example, default pitch is ignored if only pitch bend is updated.

To get MIDI-like semantics, the `keyi` (`K`) attribute should not be specified at all. If you specify pitch using `P` or `A`–`G`, then the key will be implied by the pitch, and notes will therefore be identified as in MIDI. For example, a MIDI note-off message uses key number (not pitch, which may depend upon pitch-bend, etc.) to say which note to turn off.

If the key (`K`) is specified and less than 128, and no pitch is specified, then the pitch will default to the the key number. Thus `K60` (with no `P60` or `C4` pitch specifications) is equivalent to `P60` or `C4`.

If `keyi` (`K`) is not specified, control changes will have no `keyi` and therefore apply to the channel as a whole. This also corresponds to MIDI. The one exception to this is that pressure (polyphonic aftertouch) MIDI messages need a key number. If you specify pitch along with the control change, a new note will be generated. Therefore, to issue a polyphonic aftertouch message without a new note, you should use something like `V5 KG4 -pressurer:50`, which says "on channel 5, pressure 50 on key 67 (g above middle c)."

To get more general semantics (beyond MIDI), every note can have an identifier. This allows multiple notes with the same pitch or for pitches of notes to change without ambiguity. Identifiers are specified by the `keyi` (`K`) attribute, for example `V5 K279 G4` creates a note (g above middle c) and labels it with the identifier 279. With this style, all updates should normally have a `keyi` (`K`) attribute, for example `V5 K279 -pressurer:50` sends a pressure changes to note 279. If `K` is omitted from a control change specification, then the control change applies to the channel, just as with the "MIDI style" in the previous paragraph. *In other words, there is an implicit `K-1` attribute on all non-note attribute specifications.* Thus, `keyi` is not a "sticky" attribute.

The initial default time is zero. The default time for the next event is computed as follows: Set the default time to the time of the current event. If a `N` ("N"ext) attribute is given, the default time is incremented by the value of the `N` attribute. Otherwise, if the current event is a note, increment the default time by the duration of the note. This results in a new default, which may be overridden by a `T` attribute in the next event.

### Pitch Specification

Pitches in Allegro are the same as in Adagio. The letters "A" through "G" are followed optionally by one or more "S" (sharps) or "F" (flats), and then by an octave designation (an integer). C4 is middle C. Octaves run from C to B, so B4 is higher in pitch than C4, as musicians would expect. Sharps and flats are added after the octave is computed, so for example, Cf5 is the same as B4, even though the octave numbers are different.

Octave numbers may be omitted, in which case the octave is chosen such that the pitch will be closest to the lexically previous pitch (in any voice). If the previous pitch is 6 half-steps (a tritone) away, then the current pitch will be higher than the previous pitch. When in doubt, specify the octave explicitly.

Pitch may also be specified as `P` followed by a number. `P60` is middle C, `P61` is a half step higher, equivalent to Cs4, and `P60.5` is a quarter tone sharper than P60.

Although not recommended, `P` may also be followed by a "non-P" pitch specification starting with "A" through "G".

### Duration Specification

Durations are the most complicated attributes in Allegro and Adagio. The duration letters S, I, Q, H, and W designate Sixteenth, eIghth, Quarter, Half, and Whole notes. These letters may be followed by any combination of "." (dots) and "T" (triplets). A dot multiplies the duration by 1.5. Two dots multiply the duration by 1.75, etc. A triplet multiplies the duration by 2/3. The end of the specification is an optional multiplier (an integer) followed by an optional divisor ("/" followed by an integer.)

Durations may be combined using `+`, which has lower precedence than any other duration operators.

Some examples follow:

| **Allegro Duration** | **Description** |
|----------------------|-----------------|
| `Q3` | 3 beats |
| `H.` | 3 beats |
| `HT` | 4/3 beats (half note triplet) |
| `IT.` | 1/4 beats (a dotted sixteenth triplet) |
| `HTT` | 8/9 beats |
| `Q/5` | 1/5 beats |
| `W3/23` | 12/23 beats |
| `Q..` | 7/4 beats (doubly dotted quarter) |
| `Q+I` | 3/2 beats (quarter tied to eighth) |
| `IT+Q5` | 5 and 1/3 beats (eight triplet tied to 5 quarters) |

Durations may also be specified by the letter `U` followed by a number of milliseconds. A decimal point is allowed, e.g. `U23.25`.

### Loudness Specification

Loudness is specified by `L` followed by "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff", or by a floating point number. The normal  loudness range is that of MIDI velocity, from 0 (silent) to 127 (maximum). The interpretation of dynamic markings and the numerical values is to be determined.

### Time Specification

Times are specified just like durations, except that a leading `T` indicates this is a time. If the `T` is followed by a digit, then the time is specified numerically as milliseconds. For example, `TW5` means 5 whole notes or 20 beats, while `T20` or `T20.0` means 20 milliseconds.

### Next Time Specification

The default time of the next note, sound or update can be specified just like durations, except that a leading `N` is given. If `N` is followed by a digit, then the value is specified numerically as milliseconds.  For example, `NQ` means the next event should take place a beat after this event. This specifies a default which may be overridden by an explicit time specification in the next event.

## Tempo Maps

The goal of tempo specification in Allegro is to allow flexibility without getting overly complex. There is one and only one tempo map per score. This is a limitation, but it is certainly possible to have multiple score objects when multiple tempo maps are required. Tempo maps can be specified by entering beats or by entering tempo changes. Internally, a tempo map is a sequence of pairs indicating time and beat. Tempo is assumed to be constant between any two adjacent points in the tempo map.

When the tempo map is manipulated, either the time or the beat position must change. In Allegro, changes to the tempo map also change event times so that their beat positions remain unchanged.

On the other hand, when beats are specified, the tempo map is manipulated but event times are not changed. This effectively changes the beat positions of events. This is useful if performance data is captured and then beat information is added after the fact.

The entries in the tempo map are in non-decreasing time order and non-decreasing in beat order. Any attempt to specify an illegal tempo map in Allegro generates an error. This would most likely occur in Allegro as the result of inserting a beat at a position that conflicts with other beats.

### Avoiding Numerical Problems

Instantaneous changes in beat position (infinite tempo) are not allowed, nor are zero tempos allowed. However, attempts to create an infinite or zero tempo are automatically approximated by shifting beats or times in the time map by one microsecond or one microbeat. For example, if the tempo at some beat position <i>b</i> is set to zero, the next entry in the time map is changed to have a beat position <i>b</i> + 0.000001. There is no way in Allegro to specify an infinite tempo: if you specify a second beat position at at given time, the entry in the time map is simply edited to contain the new beat position. One exception to this is at the beginning of the score. Beat 0 occurs at time 0, so if you insert, say, beat 5 at time 0, you imply an infinite tempo. The score object will automatically change this to beat 5 at time 0.000001 seconds.

### Specifying Beat Times

Beat encoding might be used to synchronize external MIDI sequences or to generate a MIDI file for use in a sequencer or music notation package where beat information is necessary. The following example specifies beat 25 at time 10.542 seconds:

```
-beatr:25.0 T10542
```

Unlike normal attribute/value pairs, which are stored as events in the score, the beatr attribute generates an entry in the tempo map. It is an error to try to insert a beat such that the implied tempo would be negative. (Beats in the tempo map must be non-decreasing.)

When a beat is inserted into the time map, events in the score retain their times. This can cause very strange behavior if the time is specified in beats, because you are saying something like "insert beat 10 at beat 15." This has a well-defined interpretation: beat 15 is mapped to time <i>t</i> in seconds. Then, the entry (<i>t</i>, 10) is inserted into the tempo map. Now, what used to be at beat 15 (or time <i>t</i>) is now at beat 10. Normally, one would only specify beats at absolute times as shown in the example above.

### Specifying Tempo

For text based notation and composition, we usually want to specify notes in terms of beats and specify the tempo of different sections of the score. The following example specifies tempo changes:

```
TQ50 -tempor:80.0
...
TQ100 -tempor:100.0
...
```

These insert tempo changes to 80 and 100 beats per minute at beats 50 and 100, respectively. Notice the use of `Q` to specify units of beats instead of milliseconds. Tempo changes can be specified at any time. Event times throughout the score are adjusted according to the new tempo.

Times and durations expressed directly in milliseconds require special care in combination with tempo specifications. When tempo is changed, <i>every</i> score event is remapped according to the new tempo. If a duration is specified as 100ms and then later in the score, tempo in that region is doubled, the actual duration will be 50ms. If the duration had been specified as a sixteenth note, it would still be a sixteenth note after the tempo change. Thus, all times and durations are treated as if they are beats when tempo
is altered.

To freely mix time and beat specifications, it is recommended to specify a complete time map before any score events.

### Time Representation

Internally, score events are associated with timestamps in units of seconds. An auxiliary structure contains a sequence of timestamped beat positions. For example, the structure might contain ((0, 0), (10, 10), (20, 30)), which indicates that at time zero, we are at beat zero. At time 10, we are at beat 10, so the tempo for the first 10 seconds is 60 beats per minute. At time 20, we are at beat 30, so the tempo from 10 to 20 is 120 beats per minute (20 beats in 10 seconds). The structure may also contain a final tempo, indicating the mapping from time to beats after the last beat position entry. If no final tempo is indicated, tempo and beats are extrapolated from the last two points in the map. Initially, the map has just one point, (0, 0), and tempo100.

There is a danger that the extrapolated tempo may not correctly predict the next beat specification. This is not an error, and it just means that the tempo over some time interval must be recomputed based on the new beat information. The problem, however, is that any events that occur within that interval should probably be adjusted in time according to the new tempo. To avoid this problem, it is best to specify timing using tempo specifications rather than beat locations. In addition, the data structure does adjust times according to the beat information. Finally, when event data is generated, both beat positions and tempo changes are sent, avoiding the need to extrapolate tempo from previous beat positions.

### Time Manipulation

An important aspect of this design is to support editing and manipulation of data. Operations include:

- Insert a beat position at a time position: this operation modifies the data structure by inserting a (time, beat) pair. No times are altered if the new beat position is not at the end of the structure. If at the end, times are adjusted between the previous last time position and this new time position.
- Insert a tempo at a time position: this inserts a (time, beat) pair and adjusts the times of everything that follows so that the tempo is achieved to the next beat. If the time position corresponds to the last time in the structure, the tempo is used in place of extrapolation.
- Maintain a tempo over a region: this inserts a (time, beat) pair for the beginning and ending of the region, and removes all pairs in the region. Times after the region are adjusted.

## Internal Representation

In [Serpent](https://www.cs.cmu.edu/~music/cmp/serpent/doc/serpent.htm), the representation is a class called `Seq`, containing the fields `Notes` and `Map`. `Notes` is a sequence of `Events`, and `Map` is a sequence of time/beat pairs.

An `Event` has fields `time`, `key`, and `channel`. Subclasses of `Event` include `Note` and `Update`. A `Note` has fields `dur`, `pitch`, `loud`, and `attributes` (attributes is a dictionary of attribute:value pairs). An `Update` has additional fields `attribute`, and `value`.

In C++, the @ref allegro.h header documents (to some extent) the data structures.
