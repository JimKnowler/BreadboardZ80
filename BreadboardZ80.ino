

// Arduino digital pins connected to Z80 pins A0...A15
int PinsAddressBus[16] = {
  53, 51, 49, 47, 45, 43, 41, 39, 37, 35, 33,
  31, 29, 27, 25, 23
};

// Arduino digital pins connected to Z80 pins D0...D7
const int PinsDataBus[8] = {
  34, 36, 30, 24, 22, 26, 28, 32
};

const int PinCLK = 2;
const int PinINT_N = 3;
const int PinNMI_N = 4;
const int PinHALT_N = 5;
const int PinMREQ_N = 6;
const int PinIORQ_N = 7;

const int PinRFSH_N = 52;
const int PinM1_N = 50;
const int PinRESET_N = 48;
const int PinBUSREQ_N = 46;
const int PinWAIT_N = 44;
const int PinBUSACK_N = 42;
const int PinWR_N = 40;
const int PinRD_N = 38;

const int PinButtonRESET = 8;
const int PinButtonSTEP = 9;

// Z80 Opcodes
const uint8_t OpcodeNOP = 0x00;

// Write a value to the data bus
void WriteDataBus(uint8_t Value)
{
  for (int i = 0; i < 8; i++)
  {
    const uint8_t PinValue = (Value >> i) & 1;
    digitalWrite(PinsDataBus[i], PinValue);
  }
}

// Read the current value from the address bus
uint16_t ReadAddressBus()
{
  uint16_t Value = 0;

  for (int i = 0; i < 16; i++)
  {
    const uint8_t PinValue = digitalRead(PinsAddressBus[i]);
    Value |= (PinValue << i);
  }

  return Value;
}

void setup() {
  Serial.begin(9600);
  Serial.println("BreadboardZ80");

  // Read from Address 
  for (const int Pin: PinsAddressBus) 
  {
    pinMode(Pin, INPUT);
  }
  
  // Write to Data Bus
  for (const int Pin: PinsDataBus)
  {
    pinMode(Pin, OUTPUT);
  }

  WriteDataBus(OpcodeNOP);

  // Write CLK
  pinMode(PinCLK, OUTPUT);
  digitalWrite(PinCLK, 0);

  // Write INT, NMI - to disable them
  pinMode(PinINT_N, OUTPUT);
  digitalWrite(PinINT_N, 1);
  pinMode(PinNMI_N, OUTPUT);
  digitalWrite(PinNMI_N, 1);

  // Read HALT, MREQ, IORQ
  pinMode(PinHALT_N, INPUT);
  pinMode(PinMREQ_N, INPUT);
  pinMode(PinIORQ_N, INPUT);

  // Read RFSH, M1
  pinMode(PinRFSH_N, INPUT);
  pinMode(PinM1_N, INPUT);

  // Write RESET, BUSREQ, WAIT
  pinMode(PinRESET_N, OUTPUT);
  digitalWrite(PinRESET_N, 1);
  pinMode(PinBUSREQ_N, OUTPUT);
  digitalWrite(PinBUSREQ_N, 1);
  pinMode(PinWAIT_N, OUTPUT);
  digitalWrite(PinWAIT_N, 1);

  // Read BUSACK, WR, RD
  pinMode(PinBUSACK_N, INPUT);
  pinMode(PinWR_N, INPUT);
  pinMode(PinRD_N, INPUT);

  // Read Buttons RESET, STEP
  pinMode(PinButtonRESET, INPUT);
  pinMode(PinButtonSTEP, INPUT);
}

void ResetZ80()
{
  Serial.println("RESET >>>> STARTING");
  digitalWrite(PinRESET_N, 0);
  delay(100);

  for (int i=0; i<5; i++)
  {
    StepZ80();
  }

  digitalWrite(PinRESET_N, 1);
  delay(100);

  Serial.println("RESET >>>> COMPLETE");
}

void StepZ80()
{
  digitalWrite(PinCLK, 1);
  delay(100);
  digitalWrite(PinCLK, 0);
  delay(100);

  const uint8_t AddressBus = ReadAddressBus();

  char buffer[128];
  snprintf(buffer, sizeof(buffer), "STEP: ADDR[%04X] M1_N[%d] RD_N[%d] WR_N[%d]", 
    AddressBus,
    digitalRead(PinM1_N),
    digitalRead(PinRD_N),
    digitalRead(PinWR_N)
  );

  Serial.println(buffer);
}

void WaitUntilRead(int Pin, int Value)
{
  while (Value != digitalRead(Pin)) {
    delay(300);
  }
}

void loop() {
  if (1 == digitalRead(PinButtonRESET))
  {
    ResetZ80();

    WaitUntilRead(PinButtonRESET, 0);
  }

  if (1 == digitalRead(PinButtonSTEP))
  {
    StepZ80();

    WaitUntilRead(PinButtonSTEP, 0);
  }
}
