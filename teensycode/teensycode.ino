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
  {.desc="C#4", .volt=0.7015},
  {.desc="D4", .volt=0.7433},
  {.desc="D#4", .volt=0.7875},
  {.desc="E4", .volt=0.8343},
  {.desc="F4", .volt=0.8839},
  {.desc="F#4", .volt=0.9364},
  {.desc="G4", .volt=0.9921},
  {.desc="G#4", .volt=1.0511},
  {.desc="A4", .volt=1.1136},
  {.desc="A#4", .volt=1.1798},
  {.desc="B4", .volt=1.2500},
  {.desc="C5", .volt=1.3243},
  {.desc="C#5", .volt=1.4031},
  {.desc="D5", .volt=1.4865},
  {.desc="D#5", .volt=1.5749},
  {.desc="E5", .volt=1.6685},
  {.desc="F5", .volt=1.7678},
  {.desc="F#5", .volt=1.8729},
  {.desc="G5", .volt=1.9843},
  {.desc="G#5", .volt=2.1022},
  {.desc="A5", .volt=2.2272},
  {.desc="A#5", .volt=2.3597},
  {.desc="B5", .volt=2.500},
  {.desc="C6", .volt=2.6487},
  {.desc="C#6", .volt=2.8062},
  {.desc="D6", .volt=2.9730},
  {.desc="D#6", .volt=3.1498},
  {.desc="E6", .volt=3.3371},
  {.desc="F6", .volt=3.5355},
  {.desc="F#6", .volt=3.7458},
  {.desc="G6", .volt=3.9685},
  {.desc="G#6", .volt=4.2045},
  {.desc="A6", .volt=4.4545},
  {.desc="A#6", .volt=4.7194},
  {.desc="B6", .volt=5.0000},
};

const float MAX_GLIDE_TIME = 1500;
float getGlideTime() {
  int val = analogRead(glideIn);
  return MAX_GLIDE_TIME * (val / 4095);
}

int NO_NOTE = numNotes;
int curNote = NO_NOTE;
float prevVolt = 0;
int prevNote = 0;
float step = 0xffff;
void updateOutput() {
  if ((curNote < 0) || (curNote > 59)) return; // don't output a note outside of proper range
  note_t note = notes[curNote];
  float desVolt = note.volt;
 
  if (curNote != prevNote && getGlideTime() && prevNote != NO_NOTE) {
    step = abs(notes[prevNote].volt - notes[curNote].volt) / (getGlideTime() / loopDt);
  } else if (!getGlideTime() || prevNote == NO_NOTE) {
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
  prevNote = curNote;
}

void OnNoteOn(byte channel, byte note, byte velocity) {
  digitalWrite(led, HIGH);
  channel -= 1;

  if (channel != 0) return;  // Keyboard should be configured to MIDI channel 0
  curNote = note - 36; // C2 is 36
}

void OnNoteOff(byte channel, byte note, byte velocity) {
  digitalWrite(led, LOW);

  curNote = NO_NOTE;
}

void setup() {
  pinMode(led, OUTPUT);
  pinMode(dacOut, OUTPUT);
  analogWriteResolution(12);
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
}

void loop() {
  usbMIDI.read();
  updateOutput();

  delay(loopDt);
}
