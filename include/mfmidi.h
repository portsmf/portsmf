#include <cstddef>

enum Alg_midi_msg_type {
    NOTEOFF = 0x80,
    NOTEON = 0x90,
    PRESSURE = 0xa0,
    CONTROLLER = 0xb0,
    PITCHBEND = 0xe0,
    PROGRAM = 0xc0,
    CHANPRESSURE = 0xd0,
};

//! These are the strings used in keynote to identify Standard MIDI File
//! meta text messages.
// FIXME: These are unused. Should they be removed?
#define METATEXT                "Text Event"
#define METACOPYRIGHT           "Copyright Notice"
#define METASEQUENCE            "Sequence/Track Name"
#define METAINSTRUMENT          "Instrument Name"
#define METALYRIC               "Lyric"
#define METAMARKER              "Marker"
#define METACUE                 "Cue Point"
#define METAUNRECOGNIZED        "Unrecognized"

//! \todo Rather than \ref finalize(), we should have ~Midifile_reader(),
//! but at least VC++ complains that there is no Mf_free(), even
//! though Mf_free is declared as virtual and this is an abstract
//! class. I don't understand this, so finalize() is a workaround. -RBD
class Midifile_reader {
public:
    void midifile();
    int Mf_nomerge = 0; //!< 1 => continue'ed system exclusives are not collapsed.
    long Mf_currtime = 0; //!< current time in delta-time units
    int Mf_skipinit = 0;   //!< 1 if initial garbage should be skipped

    //! \brief Clean up before deletion
    //!
    //! Call this method when done or you may leak memory.
    void finalize();

protected:
    int midifile_error = 0;

    virtual void *Mf_malloc(size_t size) = 0; //!< malloc()
    virtual void Mf_free(void *obj, size_t size) = 0; //!< free()
    /* Methods to be called while processing the MIDI file. */
    virtual void Mf_starttrack() = 0;
    virtual void Mf_endtrack() = 0;
    virtual int Mf_getc() = 0;
    virtual void Mf_chanprefix(int chan) = 0;
    virtual void Mf_portprefix(int p) = 0;
    virtual void Mf_eot() = 0;
    virtual void Mf_error(const char *msg) = 0;
    virtual void Mf_header(int format, int ntrks, int division) = 0;
    virtual void Mf_on(int chan, int key, int vel) = 0;
    virtual void Mf_off(int chan, int key, int vel) = 0;
    virtual void Mf_pressure(int chan, int key, int val) = 0;
    virtual void Mf_controller(int chan, int control, int val) = 0;
    virtual void Mf_pitchbend(int chan, int c1, int c2) = 0;
    virtual void Mf_program(int chan, int program) = 0;
    virtual void Mf_chanpressure(int chan, int val) = 0;
    virtual void Mf_sysex(int len, unsigned char *msg) = 0;
    virtual void Mf_arbitrary(int len, unsigned char *msg) = 0;
    virtual void Mf_metamisc(int type, int len, unsigned char *msg) = 0;
    virtual void Mf_seqnum(int n) = 0;
    virtual void Mf_smpte(int hours, int mins, int secs, int frames, int subframes) = 0;
    virtual void Mf_timesig(int i1, int i2, int i3, int i4) = 0;
    virtual void Mf_tempo(int tempo) = 0;
    virtual void Mf_keysig(int key, int mode) = 0;
    virtual void Mf_sqspecific(int len, unsigned char *msg) = 0;
    virtual void Mf_text(int type, int len, unsigned char *msg) = 0;

private:
    long Mf_toberead = 0;

    //! \brief Read a varying-length number, and return the number of
    //! characters it took.
    long readvarinum();

    long read32bit();
    int read16bit();
    void msgenlarge();
    unsigned char *msg();
    int readheader(); //!< Read a header chunk
    void readtrack(); //!< Read a track chunk
    void sysex();
    void msginit();
    int egetc(); //!< Read a single character and abort on EOF
    int msgleng();

    //! \brief Read through the "MThd" or "MTrk" header string.
    //!
    //! If \p skip == 1, attempt to skip initial garbage.
    int readmt(const char *s, int skip);
    static long to32bit(int c1, int c2, int c3, int c4);
    static int to16bit(int c1, int c2);
    void mferror(const char *s);
    void badbyte(int c);
    void metaevent(int type);
    void msgadd(int c);
    void chanmessage(int status, int c1, int c2);

    unsigned char *Msgbuff = nullptr; //!< Message buffer
    long Msgsize = 0; //!< Size of currently allocated Msg
    long Msgindex = 0; //!< Index of next available location in Msg
};
