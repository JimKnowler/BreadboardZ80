/**
 * Arduino program that acts as a debug harness around a z80 chip
 */

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

// Buttons for controlling the arduino
const int PinButtonRESET = 8;
const int PinButtonSTEP = 9;
const int PinButtonRUN = 10;
const int PinButtonInterrupt1 = 11;
const int PinButtonInterrupt2 = 12;

// Opcodes
const uint8_t OpcodeNOP = 0;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Program - compiled from asm
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>

// 01 - display a number on LED array
/*
unsigned char program_bin[] = {
  0x3e, 0xb4, 0x21, 0x00, 0x80, 0x77, 0x76
};
unsigned int program_bin_len = 7;
*/

// 02 - Multiply 2 numbers
/*
unsigned char program_bin[] = {
  0x3e, 0x00, 0x06, 0x11, 0x21, 0x00, 0x80, 0xc6, 0x0b, 0x05, 0xca, 0x10,
  0x00, 0xc3, 0x07, 0x00, 0x77, 0x76
};
unsigned int program_bin_len = 18;
*/

// 03 - Subroutines
// Expected result = 0%11010100
/*
unsigned char program_bin[] = {
  0x31, 0x00, 0x02, 0xcd, 0x07, 0x00, 0x76, 0x3e, 0x00, 0x06, 0x0f, 0xc6,
  0x0c, 0x05, 0xca, 0x14, 0x00, 0xc3, 0x0b, 0x00, 0xcd, 0x18, 0x00, 0xc9,
  0x21, 0x00, 0x80, 0x77, 0xc9
};
unsigned int program_bin_len = 29;
*/

// 04 - Interrupts

unsigned char program_bin[] = {
  0x31, 0x00, 0x02, 0xed, 0x46, 0xfb, 0x3e, 0xb7, 0xcd, 0x2f, 0x00, 0xc3,
  0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0xaa, 0xcd, 0x2f,
  0x00, 0xed, 0x4d, 0x00, 0x3e, 0x55, 0xcd, 0x2f, 0x00, 0xed, 0x4d, 0x21,
  0x00, 0x80, 0x77, 0xc9
};
unsigned int program_bin_len = 52;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Program - compiled from asm
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<

// Random Access Memory
// Memory that Z80 can read / write to
const uint16_t kRamStartAddress = 0x100;
const uint16_t kRamEndAddress = 0x200;
uint8_t RAM[256];

// Interrupt
// Opcode that will be written to the databus when an interrupt is acknowledged
// NOTE: assumes Z80 is using interrupt mode 0
uint8_t InterruptOpcode = 0;

// Control Flags
namespace {
  bool bIsRunning = false;
  bool bIsResetting = false;
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
  bIsResetting = true;

  digitalWrite(PinRESET_N, 0);
  delay(100);

  // step 3 cycles when reset pin is active
  for (int i=0; i<3; i++)
  {
    StepZ80();
  }

  digitalWrite(PinRESET_N, 1);
  delay(100);

  // step another 2 cycles after reset pin is inactive, to skip CPU finishing reset
  for (int i=0; i<2; i++)
  {
    StepZ80();
  }

  bIsResetting = false;
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
  const bool bIORQ = (digitalRead(PinIORQ_N) == 0);
  const bool bM1 = (digitalRead(PinM1_N) == 0);

  if (bHALT)
  {
    bIsRunning = false;
  }

  uint8_t DataBus = 0x00;

  if ((AddressBus >= kRamStartAddress) && (AddressBus < kRamEndAddress))
  {
    // RAM access
    if (bWR)
    {
      SetupDataBusRead();
      DataBus = ReadDataBus();

      RAM[AddressBus - kRamStartAddress] = DataBus;
    } else if (bRD) {
      SetupDataBusWrite();
      DataBus = RAM[AddressBus - kRamStartAddress];
      WriteDataBus(DataBus);
    }
  }
  else if (bWR)
  {
    // writing outside of RAM

    SetupDataBusRead();
    DataBus = ReadDataBus();
  } else {
    SetupDataBusWrite();

    if (bIORQ && bM1)
    {
      // interrupt is being acknowledged
      // we're using Interrupt Mode 0, so we need to write an opcode to the data bus

      // write interrupt opcode to data bus
      DataBus = InterruptOpcode;
      WriteDataBus(DataBus);

      // disable the interrupt pin
      digitalWrite(PinINT_N, 1);
    }
    else if (bRD)
    {
      if (AddressBus < program_bin_len) {
        // read program
        DataBus = program_bin[AddressBus];
        WriteDataBus(DataBus);
      } else if (!bIsResetting) {
        // invalid read
        char Buffer[128];
        snprintf(Buffer, sizeof(Buffer), "ERROR: trying to read to address [%04X] when program only has [%04X] bytes", AddressBus, program_bin_len);
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
  snprintf(buffer, sizeof(buffer), "STEP: ADDR[%04X] DATA[%02X] %s %s %s %s %s %s %s %s %s %s %s", 
    AddressBus,
    DataBus,
    bM1 ? "M1" : "  ",
    bRD ? "RD" : "  ",
    bWR ? "WR" : "  ",
    (digitalRead(PinMREQ_N) == 0) ? "MREQ" : "    ",
    bIORQ ? "IORQ" : "    ",
    (digitalRead(PinBUSACK_N) == 0) ? "BUSACK" : "      ",
    (digitalRead(PinRFSH_N) == 0) ? "RFSH" : "    ",
    bHALT ? "HALT" : "    ",
    (digitalRead(PinWAIT_N) == 0) ? "WAIT" : "    ",
    (digitalRead(PinINT_N) == 0) ? "INT" : "   ",
    (digitalRead(PinNMI_N) == 0) ? "NMI" : "   "
  );

  Serial.println(buffer);
}

// NOTE: InterruptHandlerAddress must be divisible by 8
// NOTE: InterruptHandlerAddress should be in range of 0x00 to 0x38
void StartInterrupt(uint8_t InterruptHandlerAddress)
{
  // start the sequence of issuing an interrupt, assuming that the CPU is using Interrupt Mode 0


  //
  // TODO: debug this opcode - is it valid?
  //
  
  const uint8_t OpcodeRST = 0b11000111 | InterruptHandlerAddress;

  char Buffer[16];
  snprintf(Buffer, sizeof(Buffer), "int opcode 0x%02x", OpcodeRST);
  Serial.println(Buffer);

  digitalWrite(PinINT_N, 0);
  InterruptOpcode = OpcodeRST;
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

  if (1 == digitalRead(PinButtonInterrupt1))
  {
    bIsRunning = false;
    Serial.println("Interrupt 1");

    WaitUntilRead(PinButtonInterrupt1, 0);

    StartInterrupt(0x20);
  }

  if (1 == digitalRead(PinButtonInterrupt2))
  {
    bIsRunning = false;
    Serial.println("Interrupt 2");

    WaitUntilRead(PinButtonInterrupt2, 0);

    StartInterrupt(0x28);
  }

  if (bIsRunning)
  {
    StepZ80();
  }

}
