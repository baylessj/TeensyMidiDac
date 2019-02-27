int ledPin = 13;

void OnNoteOn(byte channel, byte note, byte velocity)
{digitalWrite(ledPin, HIGH);}

void OnNoteOff(byte channel, byte note, byte velocity)
{digitalWrite(ledPin, LOW);}

void setup()
{
  pinMode(ledPin, OUTPUT);
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn) ;
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin, LOW);
}

void loop()
{
  usbMIDI.read();
}
