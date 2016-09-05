

 #include "stm32f4xx.h"

//functions to initializing I/O pins and read/write data from pins
void initPinsToPlis(void);
void setAddressToPlis(unsigned char addr);
void resetAddressToPlis(void);
void setWriteDataToPlis(unsigned char dataToWrite);
void resetWriteDataToPlis(void);
unsigned char readDataByteFromPlis(void);
unsigned char readDataByteFromPlis(void);


//Block of functions to exchange with PLIS
void writeDataOnAddrPlis(unsigned char addr, unsigned char dataByte);
unsigned char getDataByteFromPlis(unsigned char address);


//Bit set or reset and get functions
void setBitInUInt32(unsigned long int *pWord32, unsigned char bitValue, unsigned char bitNumber);
void setBitInUInt16(unsigned int *pWord16, unsigned char bitValue, unsigned char bitNumber);
void setBitInUChar(unsigned char *pUChar, unsigned char bitValue, unsigned char bitNumber);
void setBitInByte(unsigned char *pByte, unsigned char bitNumber);
void resetBitInByte(unsigned char *pByte, unsigned char bitNumber);
unsigned char getBitFromByte(unsigned char byte, unsigned char numberOfBit);


//Functions to control of coordinate detectors
void permResetZCoordDetect(unsigned char detectorNumb, unsigned char permission);
void permResetZCoordDetectColona(unsigned char permission);
void permResetZCoordDetectKaretka(unsigned char permission);
void permResetZCoordDetectAzimut(unsigned char permission);

void resetCoordDetect(unsigned char detectorNumb);
void resetCoordDetectColona();
void resetCoordDetectKaretka();
void resetCoordDetectAzimut();

void setCurCoordDet(unsigned char detectorNumb, unsigned char permission, unsigned char reset);
void setCurCoordDetColona(unsigned char permission, unsigned char reset);
void setCurCoordDetKaretka(unsigned char permission, unsigned char reset);
void setCurCoordDetAzimut(unsigned char permission, unsigned char reset);

unsigned char getActiveCoordDetect();
unsigned char getPermResCurCoordDet(unsigned char detectorNumb);


//Functions to control of PLIS Comparators
void dirOfADCComp(unsigned char direction);
void dirOfMotorComp(unsigned char direction);
void dirOfSlowSpeedComp(unsigned char direction);

void permOfADCComp(unsigned char permission);
void permOfMotorComp(unsigned char permission);
void permOfSlowSpeedComp(unsigned char permission);
void setComparatorsOptions(unsigned char dir1, unsigned char per1, unsigned char dir2, unsigned char per2, unsigned char dir3, unsigned char per3);

unsigned char getDirOfADCComp();
unsigned char getDirOfMotorComp();
unsigned char getDirOfSlowSpeedComp();
unsigned char getPermOfADCComp();
unsigned char getPermOfMotorComp();
unsigned char getPermOfSlowSpeedComp();


//Functions to writing the coordinates for comparators
void writeCoordToADCComp(long int coord);
void writeCoordToMotorComp(long int coord);
void writeCoordToSlowSpeedComp(long int coord);


//Functions for motors controlling
unsigned char getEngNumbFromDetNumb(unsigned char detectNumb);
unsigned char getDetNumbFromEngNumb(unsigned char engineNumb);
void stopEngines();
void switchEngine(unsigned char engineNumb);
unsigned char getActiveEngine();
void switchEngineUgolMesta();
void switchEnginePeremesch();
void switchEngineAzimut();
void switchEngineFokal();
void switchEnginePodyom();
void switchEngineKolona();
void switchEngineKaretka();


//Functions for controlling motor control unit
void motionDirectOfEngine(signed char direction);
void slowSpeedSwitch(unsigned char toggle);
void switchEngineSpeed(unsigned char speedCode);
void switchBrakingMode(unsigned char brakingMode);

unsigned char getBrakingMode();
unsigned char getEngineSpeed();


//Functions for controlling switches VV88M
void setVV88MFromDKS(unsigned char switchNumb);
void setVV88MFromZond(unsigned char switchNumb);
unsigned char getSwitchWorkMode(unsigned char switchNumb);


//Functions for reading special datas from PLIS
long int getCurrCoordFromDetect();
unsigned char getStateBitADCReq();
unsigned char getStateBitStopReq();
unsigned char getStateBitSlowReq();
unsigned char getStateBitVd0();
unsigned char getStateBitVd1();
unsigned char getStateBitPermZeroReset();
unsigned char getStateBitEVMOrMest();
unsigned char getStateBitATVReady();
unsigned char getStateBitATVCrash();
unsigned char getStateBitATVBlocking();
unsigned char getStateBitStopR();
unsigned char getStateBitStopL();
unsigned char getStateBitDriveR();
unsigned char getStateBitDriveL();
unsigned char getStateBitSlowSpeed();
unsigned char getStateBitEngUgolMesta();
unsigned char getStateBitEngPeremesch();
unsigned char getStateBitEngAzimut();
unsigned char getStateBitEngFokal();
unsigned char getStateBitEngPodyom();
unsigned char getStateEngColona();
unsigned char getStateEngKaretka();

unsigned char getActiveEngineFromPLIS();


//Functoin to check connection with PLIS
unsigned char dataExchangeTest(unsigned char data);


