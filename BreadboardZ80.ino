
// Arduino digital pins connected to Z80 pins A0...A15
int PinsAddressBus[16] = {
  53, 51, 49, 47, 45, 43, 41, 39, 37, 35, 33,
  31, 29, 27, 25, 23
};

// Arduino digital pins connected to Z80 pins D0...D7
const int PinsDataBus[8] = {
  34, 36, 30, 24, 22, 26, 28, 32
};

const int PinCLK_N = 2;
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

void setup() {
  Serial.begin(9600);
  Serial.println("BreadboardZ80");

  

}

void loop() {
  // put your main code here, to run repeatedly:

}
