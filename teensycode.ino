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
  {.desc="C2", .volt=0.1655},
  {.desc="C#2", .volt=0.1754},
  {.desc="D2", .volt=0.1858},
  {.desc="D#2", .volt=0.1969},
  {.desc="E2", .volt=0.2086},
  {.desc="F2", .volt=0.2210},
  {.desc="F#2", .volt=0.2341},
  {.desc="G2", .volt=0.2480},
  {.desc="G#2", .volt=0.2628},
  {.desc="A2", .volt=0.2784},
  {.desc="A#2", .volt=0.2950},
  {.desc="B2", .volt=0.3125},
  {.desc="C3", .volt=0.3311},
  {.desc="C#3", .volt=0.3508},
  {.desc="D3", .volt=0.3716},
  {.desc="D#3", .volt=0.3937},
  {.desc="E3", .volt=0.4171},
  {.desc="F3", .volt=0.4419},
  {.desc="F#3", .volt=0.4682},
  {.desc="G3", .volt=0.4961},
  {.desc="G#3", .volt=0.5256},
  {.desc="A3", .volt=0.5568},
  {.desc="A#3", .volt=0.5899},
  {.desc="B3", .volt=0.6250},
  {.desc="C4", .volt=0.6622},
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

int noteMsg = 60; // larger than possible not values
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

  if ((noteMsg < 0) || (noteMsg > 59)) continue;  // Only 60 notes of keyboard are supported
  commandNote(noteMsg);
  
  analogWrite(dacOut, 2000); // test output for the DAC, range is 0-4095 (this is unique to the teensy)

  delay(loopDt);
}
