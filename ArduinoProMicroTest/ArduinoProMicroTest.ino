
#include <SPI.h>

#define sSS 2
#define sCLK 3
#define sDOUT 4
#define sDIN 5

#define sZERO 7
#define sCF 8
#define sPWCHK 9
//static SoftSPI<sDOUT, sDIN, sCLK, 2> spi;

void setup()
{
  SPI.begin();
  pinMode(sSS,OUTPUT);
  digitalWrite(sSS, HIGH);//disabled by default
  // SPI Init
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE2));
  delay(10);
  
  pinMode(sZERO, INPUT);
  pinMode(sCF, INPUT);
  pinMode(sPWCHK, INPUT);
  
  Serial.begin(250000);
}

void setupADE7753() {
  EIMSK &= ~_BV(INT6); // disable interrupt
  
  digitalWrite(sSS, LOW);
  SPI.transfer(0x89); // 09 MODE Write 16 Bit Unsigned default 0x000C
  // Mode Register. This is a 16-bit register through which most of the ADE7753
  // functionality is accessed. Signal sample rates, filter enabling, and
  // calibration modes are selected by writing to this register. The contents can
  // be read at any time—see the Mode Register (0x9) section. 
  SPI.transfer(0x80);
  // Bit 12,11 = DTRT1, 0 = These bits are used to select the waveform register update rate. 
  SPI.transfer(0x0c);
  digitalWrite(sSS, HIGH);
  delayMicroseconds(100);

  digitalWrite(sSS, LOW);
  SPI.transfer(0x98); // 18 IRMSOS Write 12 Bit Signed
  // Channel 1 RMS Offset Correction Register. 
  SPI.transfer(0x0f);
  SPI.transfer(0x15);
  digitalWrite(sSS, HIGH);
  delayMicroseconds(100);

  digitalWrite(sSS, LOW);
  SPI.transfer(0x8f); // 0f GAIN Write 8 Bit Unsigned
  // PGA Gain Adjust. This 8-bit register is used to adjust the gain selection for
  // the PGA in Channels 1 and 2—see the Analog Inputs section. 
  SPI.transfer(0x10);
  // 3 bits PGA2 GAIN select (000) = 1
  // 2 bits channel 1 full-scale select (10) = 0.125V
  // 3 bits PGA1 GAIN select (000) = 1
  digitalWrite(sSS, HIGH);
  delayMicroseconds(100);

  digitalWrite(sSS, LOW);
  SPI.transfer(0x94); // 14 CFNUM Write 12 Bit Unsigned default 0x3f
  // CF Frequency Divider Numerator Register. The output frequency on the CF
  // pin is adjusted by writing to this 12-bit read/write register—see the Energy to-Frequency
  // Conversion section.
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  digitalWrite(sSS, HIGH);
  delayMicroseconds(100);

  digitalWrite(sSS, LOW);
  SPI.transfer(0x95); // 15 CFDEN Write 12 Bit Unsigned default 0x3f
  // CF Frequency Divider Denominator Register. The output frequency on the
  // CF pin is adjusted by writing to this 12-bit read/write register—see the
  // Energy-to-Frequency Conversion section. 
  SPI.transfer(0x08);
  SPI.transfer(0x75);
  digitalWrite(sSS, HIGH);

  EICRB |= _BV(ISC61);
  EICRB &= ~_BV(ISC60); // Rising edge
  EIMSK |= _BV(INT6); // activates the interrupt
}

#define READINGS 100

volatile unsigned long lastMillis;
volatile byte doread = 1;
volatile unsigned long watt;
volatile unsigned long amps;
volatile unsigned long volts;
volatile int period;

volatile unsigned long rWatt[READINGS];
volatile unsigned long rAmps[READINGS];
volatile unsigned long rVolts[READINGS];
volatile int rIndex = 0;

ISR(INT6_vect) {
  lastMillis = millis();
  watt = readSpi(0x03, 3);  // RAENERGY 24 Bit readonly Signed
  amps = readSpi(0x16, 3);  // IRMS 24 Bit readonly Unsigned
  delayMicroseconds(900);
  volts = readSpi(0x17, 3); // VRMS 24 Bit readonly Unsigned
  delayMicroseconds(900);
  period = (int)readSpi(0x27, 2); // PERIOD 16 Bit readonly Unsigned

  rIndex++;
  if (rIndex >= READINGS) rIndex = 0;
  rWatt[rIndex] = watt;
  rAmps[rIndex] = amps;
  rVolts[rIndex] = volts;
  
  doread = 1;
}

unsigned long readSpi(byte reg, int bytesToRead) {
  delayMicroseconds(10);
  digitalWrite(sSS, LOW);
  delayMicroseconds(2);
  SPI.transfer(reg);
  unsigned long result = 0;
  //delayMicroseconds(25);
  for (int i = 0; i < bytesToRead; i++) {
    byte b = SPI.transfer(0xff);
    result = result << 8 | b;
  }
  delayMicroseconds(4);
  digitalWrite(sSS, HIGH);  
  return result;
}

unsigned long last = 0;
bool lastpwchk = false;
unsigned long lastpwchkok = 0;
bool lastpwchkinit = false;
void loop()
{
  // read CF and PWCHK
  bool zx = PINE & B01000000;
  bool cf = PINB & B00010000;  // B4 digitalRead(sCF); // 
  bool pwchk = PINB & B00100000;  // B5 digitalRead(sPWCHK); // 
  unsigned long now = millis();

  bool pwchkChange = lastpwchk != pwchk;
  if (pwchkChange) {
    // TODO somehow ignore minimal glitches!
    lastpwchk = pwchk;
    if (pwchk) lastpwchkok = now;
    lastpwchkinit = false;
  }
  if (pwchkChange || cf)
  {
    Serial.print(now);
    Serial.print(" ");
    Serial.print(pwchk, BIN);
    Serial.print(pwchkChange, BIN);
    Serial.print(cf, BIN);
    Serial.print(" ");
    //Serial.print(PINE, BIN);
    //Serial.print(" ");
    Serial.print(PINB, BIN);
    Serial.print("  ");
    if (cf) {
      Serial.println(" .. We got a CF! .. ");
      delay(10000);
    }
    if (doread == 0) {
      Serial.println();
      delay(1000);
    }
  }

  // make sure that we dont run stuffs until we are ok
  if (!pwchk || now - lastpwchkok < 1000) return;
  if (!lastpwchkinit) {
    Serial.println("doing init ...");
    lastpwchkinit = true;
    setupADE7753();
  }

  if (doread == 0) return;
  if (now - last < 1000) return;

  Serial.print(now);
  Serial.print(" ");
  // sync on ZX (Zero Crossing)
  /* int state = 0;
  digitalWrite(sSS, LOW);
  spi.transfer(0x0C);  // RSTATUS 16 Bit readonly Unigned
  state = spi.transfer(0xff) << 8 | spi.transfer(0xff);
  digitalWrite(sSS, HIGH);

  while(! state & 0x0010) { // ZX
    digitalWrite(sSS, LOW);
    spi.transfer(0x0B);  // STATUS 16 Bit readonly Unigned
    state = spi.transfer(0xff) << 8 | spi.transfer(0xff);
    digitalWrite(sSS, HIGH);
  }*/

  unsigned long long totWatts = 0;
  unsigned long long avgAmps = 0;
  unsigned long long avgVolts = 0;
  for (int i = 0; i < READINGS; i++) {
    totWatts += rWatt[i];
    avgAmps += rAmps[i];
    avgVolts += rVolts[i];
  }

  unsigned long tdiff = now - last;
  // TODO get proper TDIFF for the oldest item in array
  //unsigned long watt = readSpi(0x03, 3);  // RAENERGY 24 Bit readonly Signed
  float cwatt = ((float)totWatts / READINGS) * 25.333333; // (5 / (float)tdiff);
  last = now;
  // Active Energy Register. Active power is in this 24-bit, read-only register,
  // reset to 0 following a read operation. 
  // see the Energy Calculation section. 
  //Serial.print(watt);
  //Serial.print(" ");
  Serial.print(cwatt);
  // Tests show that if this is taken at 0.5Hz then it's close to correct value
  Serial.print("W ");

  // 0x120000 is a hack!!!
  //unsigned long amps = readSpi(0x16, 3) & ~0x120000; // IRMS 24 Bit readonly Unsigned
  // Channel 1 RMS Value (Current Channel).
  Serial.print(" ");
  Serial.print(amps, HEX);
  // TODO sync ZX and do AVG
  Serial.print("\t =");
  Serial.print((float)(avgAmps / READINGS) / 54123.0);
  Serial.print("A ");

  //unsigned long volts = readSpi(0x17, 3); // VRMS 24 Bit readonly Unsigned
  // Channel 2 RMS Value (Voltage Channel). 
  Serial.print(" ");
  Serial.print(volts, HEX);
  // TODO sync ZX and do AVG
  Serial.print("\t =");
  Serial.print((float)(avgVolts / READINGS) / 5015.0);
  Serial.print("V ");

  const float fqDiv = 3579545 / 8; // 8 = 4 / 2;   // 2 = 32 * 16
  //int period = (int)readSpi(0x27, 2); // PERIOD 16 Bit readonly Unsigned
  // Period of the Channel 2 (Voltage Channel) Input Estimated by ZeroCrossing
  // Processing. The MSB of this register is always zero. 
  // when CLKIN = 3.579545 MHz, which represents 0.013% when the line frequency
  // is 60 Hz When the line frequency is 60 Hz, the value of
  // the period register is approximately CLKIN/4/32/60 Hz * 16 =
  // 7457d. The length of the register enables the measurement of line
  // frequencies as low as 13.9 Hz. 
  Serial.print(period, HEX);
  Serial.print(" =");
  Serial.print(fqDiv / (float)period);
  Serial.print("Hz ");
  doread = 0;

  Serial.println(" ");
}