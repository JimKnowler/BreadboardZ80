

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
const int PinButtonRUN = 10;

// Z80 Opcodes
const uint8_t OpcodeNOP = 0x00;
const uint8_t OpcodeLD_A_Immediate = 0x3E;        // Load 8 bit immediate value into A, followed by a 1 x byte for immediate value
const uint8_t OpcodeADD_A_Immediate = 0xC6;       // Add 8 bit immediate value to A, followed by 1 x byte for immediate value
const uint8_t OpcodeLD_HL_Immediate = 0x21;       // Load 16 bit immediate value into HL, followed by 2 x bytes for immediate value
const uint8_t OpcodeLD_HL_A = 0x77;               // Store 8 bit value in A to address in HL
const uint8_t OpcodeHALT = 0x76;                  // HALT operation of the CPU

// Program - Increment a value and write it to memory
const uint8_t Program[] = {
  OpcodeLD_A_Immediate, 0xAB,                     // initialize value of A
  OpcodeADD_A_Immediate, 0x01,                    // increment value of A
  OpcodeLD_HL_Immediate, 0x34, 0x12,              // initialize HL with address 0x1234 (the address that we will write our result to)
  OpcodeLD_HL_A,                                  // write result to address in HL
  OpcodeHALT
};

// Control Flags
namespace {
  bool bIsRunning = false;
}

// Write a value to the data bus
void WriteDataBus(uint8_t Value)
{
  for (int i = 0; i < 8; i++)
  {
    const uint8_t PinValue = (Value & (1 << i)) != 0;
    digitalWrite(PinsDataBus[i], PinValue);
  }
}

// Read a value from the data bus
uint8_t ReadDataBus()
{
  uint8_t Value = 0;

  for (int i = 0; i < 8; i++)
  {
    const uint8_t PinValue = digitalRead(PinsDataBus[i]);
    Value |= (PinValue << i);
  }

  return Value;
}

// Read the current value from the address bus
uint16_t ReadAddressBus()
{
  uint16_t Value = 0;

  for (int i = 0; i < 16; i++)
  {
    const uint16_t PinValue = digitalRead(PinsAddressBus[i]);
    Value |= (PinValue << i);
  }

#if DEBUG_READ_ADDRESSBUS
  char Buffer[128];
  snprintf(Buffer, sizeof(Buffer),"ReadAddressBus [%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d] => %04X",
    digitalRead(PinsAddressBus[15]),
    digitalRead(PinsAddressBus[14]),
    digitalRead(PinsAddressBus[13]),
    digitalRead(PinsAddressBus[12]),
    digitalRead(PinsAddressBus[11]),
    digitalRead(PinsAddressBus[10]),
    digitalRead(PinsAddressBus[9]),
    digitalRead(PinsAddressBus[8]),
    digitalRead(PinsAddressBus[7]),
    digitalRead(PinsAddressBus[6]),
    digitalRead(PinsAddressBus[5]),
    digitalRead(PinsAddressBus[4]),
    digitalRead(PinsAddressBus[3]),
    digitalRead(PinsAddressBus[2]),
    digitalRead(PinsAddressBus[1]),
    digitalRead(PinsAddressBus[0]),
    Value
  );

  Serial.println(Buffer);
#endif

  return Value;
}

void ResetZ80()
{
  Serial.println("RESET >>>> STARTING");
  digitalWrite(PinRESET_N, 0);
  delay(100);

  for (int i=0; i<8; i++)
  {
    StepZ80();
  }

  digitalWrite(PinRESET_N, 1);
  delay(100);

  Serial.println("RESET >>>> COMPLETE");
}

void SetupDataBusWrite()
{
  for (const int Pin: PinsDataBus)
  {
    pinMode(Pin, OUTPUT);
  }
}

void SetupDataBusRead()
{
  for (const int Pin: PinsDataBus)
  {
    pinMode(Pin, INPUT);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("BreadboardZ80");

  // Read from Address 
  for (const int Pin: PinsAddressBus) 
  {
    pinMode(Pin, INPUT);
  }
  
  // Write to Data Bus
  SetupDataBusWrite();
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

  ResetZ80();
}

void StepZ80()
{
  digitalWrite(PinCLK, 1);
  delay(1);
  digitalWrite(PinCLK, 0);
  delay(1);

  const uint16_t AddressBus = ReadAddressBus();

  const bool bRD = (digitalRead(PinRD_N) == 0);
  const bool bWR = (digitalRead(PinWR_N) == 0);
  const bool bHALT = (digitalRead(PinHALT_N) == 0);

  if (bHALT)
  {
    bIsRunning = false;
  }

  uint8_t DataBus = 0x00;

  if (bWR)
  {
    SetupDataBusRead();
    DataBus = ReadDataBus();
  } else {
    SetupDataBusWrite();
    if (bRD)
    {
      const uint16_t ProgramSize = sizeof(Program);
      if (AddressBus < ProgramSize) {
        DataBus = Program[AddressBus];
        WriteDataBus(DataBus);
      } else {
        char Buffer[128];
        snprintf(Buffer, sizeof(Buffer), "ERROR: trying to read to address [%04X] when program only has [%04X] bytes", AddressBus, ProgramSize);
        Serial.println(Buffer);

        bIsRunning = false;
      }
    }
    else
    {
      WriteDataBus(0x00);
    }
  }

  char buffer[128];
  snprintf(buffer, sizeof(buffer), "STEP: ADDR[%04X] DATA[%02X] %s %s %s %s %s %s %s %s", 
    AddressBus,
    DataBus,
    (digitalRead(PinM1_N) == 0) ? "M1" : "  ",
    bRD ? "RD" : "  ",
    bWR ? "WR" : "  ",
    (digitalRead(PinMREQ_N) == 0) ? "MREQ" : "    ",
    (digitalRead(PinIORQ_N) == 0) ? "IORQ" : "    ",
    (digitalRead(PinBUSACK_N) == 0) ? "BUSACK" : "      ",
    (digitalRead(PinRFSH_N) == 0) ? "RFSH" : "    ",
    bHALT ? "HALT" : "    "
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
    bIsRunning = false;

    ResetZ80();

    WaitUntilRead(PinButtonRESET, 0);
  }

  if (1 == digitalRead(PinButtonSTEP))
  {
    bIsRunning = false;

    StepZ80();

    WaitUntilRead(PinButtonSTEP, 0);
  }

  if (1 == digitalRead(PinButtonRUN))
  {
    bIsRunning = true;
  }

  if (bIsRunning)
  {
    StepZ80();
  }

}
