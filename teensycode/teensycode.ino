/**
 *  MIDI interface with a Teensy 3.2
 *  
 *  Allows for input from a MIDI keyboard over USB and output over the Teensy's DAC
 */
const int led = 13;
const int dacOut = A14;
const int glideIn = A8;

const int loopDt = 50; // in ms

static const int numNotes = 60;

typedef struct {
  const char* desc;
  float volt;
} note_t;

note_t notes[numNotes] = {
  {.desc="C2", .volt=0.382},
  {.desc="C#2", .volt=0.405},
  {.desc="D2", .volt=0.429,
  {.desc="D#2", .volt=0.455},
  {.desc="E2", .volt=0.482},
  {.desc="F2", .volt=0.511},
  {.desc="F#2", .volt=0.542},
  {.desc="G2", .volt=0.574},
  {.desc="G#2", .volt=0.609},
  {.desc="A2", .volt=0.646},
  {.desc="A#2", .volt=0.685},
  {.desc="B2", .volt=0.726},
  {.desc="C3", .volt=0.771},
  {.desc="C#3", .volt=0.817},
  {.desc="D3", .volt=0.867},
  {.desc="D#3", .volt=0.919},
  {.desc="E3", .volt=0.975},
  {.desc="F3", .volt=1.035},
  {.desc="F#3", .volt=1.099},
  {.desc="G3", .volt=1.166},
  {.desc="G#3", .volt=1.236},
  {.desc="A3", .volt=1.311},
  {.desc="A#3", .volt=1.393},
  {.desc="B3", .volt=1.479},
  {.desc="C4", .volt=1.571},
};

const float MAX_GLIDE_TIME = 1500;
float getGlideTime() {
  int val = analogRead(glideIn);
  return 1500 * (val / 4095);
}

float prevVolt = 0;
int prevNote = 0;
float step = 0xffff;
void commandNote(int inote) {
  note_t note = notes[inote];
  float desVolt = note.volt;
 
  if (inote != prevNote && getGlideTime()) {
    step = abs(notes[prevNote].volt - notes[inote].volt) / (getGlideTime() / loopDt);
  } else if (!getGlideTime()) {
    // no slew
    step = 0xffff;
  }

  // Slew according to the glide param
  if (desVolt - prevVolt > step) {
    desVolt = prevVolt + step;
  } else if (desVolt - prevVolt < -step) {
    desVolt = prevVolt - step;
  }
  
  uint16_t cmdVolt = (desVolt / 5.0) * 4095; // convert to 0-4095 range
  analogWrite(dacOut, cmdVolt);
  
  prevVolt = desVolt;
  prevNote = inote;
}

void setup() {
  pinMode(led, OUTPUT);
  pinMode(dacOut, OUTPUT);
  analogWriteResolution(12);
}

int noteMsg = 60; // larger than possible note values
void loop() {
  if (usbMIDI.read()) {
    digitalWrite(led, HIGH);

    byte type = usbMIDI.getType();
    switch (type) {
        
      case usbMIDI.NoteOff:
        break;
      case usbMIDI.NoteOn:
        noteMsg = usbMIDI.getData1() - 21; // A0 = 21, Top Note = 108
        int channel = usbMIDI.getChannel() - 1;

        if (channel != 0) return;  // Keyboard should be configured to MIDI channel 0
        break;
    }
  } else {
    digitalWrite(led, LOW);
  }

  if ((noteMsg >= 0) && (noteMsg < 60))  // Only 60 notes of keyboard are supported
    commandNote(noteMsg);
  
  analogWrite(dacOut, 2000); // test output for the DAC, range is 0-4095 (this is unique to the teensy)

  delay(loopDt);
}
