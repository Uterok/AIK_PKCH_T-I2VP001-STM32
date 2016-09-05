

 #include "plis_command.h"

//constants
const unsigned char COUNT_OF_COORD_DETECT = 3;

//global variables
//variables of state
struct coord_detect {
  unsigned char active;
  unsigned char regOfCtrlCoordDetect[3];
};
struct coord_detect coordDetect;
unsigned char regOfCtrlCompar = (unsigned char)0x00;
unsigned char regOfSwitchEngines = (unsigned char)0x00;
unsigned char regOfCtrlEngines[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char regOfVV88MStates = (unsigned char)0x00;


//-------------------------------------Block of functions to initializing I/O pins and read/write data from pins-----------------------------------------------------
void initPinsToPlis()
{
  GPIO_InitTypeDef gpioConfOut;
  GPIO_InitTypeDef gpioConfIn;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  
  //Initializing pins of address bus
  //Set overall mode of pins
  gpioConfOut.GPIO_Mode = GPIO_Mode_OUT;
  gpioConfOut.GPIO_Speed = GPIO_Speed_100MHz;
  gpioConfOut.GPIO_OType = GPIO_OType_PP;
  gpioConfOut.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  //Set and initializing each pin
  gpioConfOut.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_2;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_4;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_8;
  GPIO_Init(GPIOD, &gpioConfOut);
  
  //Initializing pins of data bus on write(overall mode leave unchanged)
  gpioConfOut.GPIO_Pin = GPIO_Pin_8;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_11;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_12;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_13;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_14;
  GPIO_Init(GPIOE, &gpioConfOut);
  gpioConfOut.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOE, &gpioConfOut);
  
  //Initializing pins of data bus on read
  //Set mode to out
  gpioConfIn.GPIO_Mode = GPIO_Mode_IN;
  
  gpioConfIn.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOD, &gpioConfIn);
  gpioConfIn.GPIO_Pin = GPIO_Pin_1;
  GPIO_Init(GPIOD, &gpioConfIn);
  gpioConfIn.GPIO_Pin = GPIO_Pin_2;
  GPIO_Init(GPIOD, &gpioConfIn);
  gpioConfIn.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOD, &gpioConfIn);
  gpioConfIn.GPIO_Pin = GPIO_Pin_4;
  GPIO_Init(GPIOD, &gpioConfIn);
  gpioConfIn.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOD, &gpioConfIn);
  gpioConfIn.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOD, &gpioConfIn);
  gpioConfIn.GPIO_Pin = GPIO_Pin_1;
  GPIO_Init(GPIOE, &gpioConfIn);
}

void setAddressToPlis(unsigned char addr)
{
  
  GPIOD->BSRRH |= 0x0100;
  
  while(((GPIOD->ODR >> 8) & 0x00000001))
  ;

  if((addr & 0x01) > 0)
  {
    GPIOE->BSRRL |= 0x0001;
  }
  else
  {
    GPIOE->BSRRH |= 0x0001;
  }
  
  if((addr & 0x02) > 0)
  {
    GPIOE->BSRRL |= 0x0004;
  }
  else
  {
    GPIOE->BSRRH |= 0x0004;
  }
  
  if((addr & 0x04) > 0)
  {
    GPIOE->BSRRL |= 0x0010;
  }
  else
  {
    GPIOE->BSRRH |= 0x0010;
  }
  
  if((addr & 0x08) > 0)
  {
    GPIOE->BSRRL |= 0x0020;
  }
  else
  {
    GPIOE->BSRRH |= 0x0020;
  }
  
  while((unsigned long int)((GPIOE->ODR >> 5) & 0x00000001) != (unsigned long int)((addr >> 3) & 0x01))
  ;
  
  GPIOD->BSRRL |= 0x0100;
  GPIOD->BSRRH |= 0x0100;
  
  while(((GPIOD->ODR >> 8) & 0x00000001))
  ;
  
  if((addr & 0x10) > 0)
  {
    GPIOE->BSRRL |= 0x0001;
  }
  else
  {
    GPIOE->BSRRH |= 0x0001;
  }
  
  GPIOE->BSRRH |= 0x0004;
  GPIOE->BSRRH |= 0x0010;
  GPIOE->BSRRH |= 0x0020;
  
  while((unsigned long int)((GPIOE->ODR >> 5) & 0x00000001) != (unsigned long int)0)
  ;
  
  GPIOD->BSRRL |= 0x0100;
  GPIOD->BSRRH |= 0x0100;
  
  
  //unsigned long int regPE_ODR = GPIOE->ODR;
  
  
  //setBitInUInt32(&regPE_ODR, (addr >> 3) & (unsigned char)0x01, (unsigned char)5);
  //setBitInUInt32(&regPE_ODR, (addr >> 2) & (unsigned char)0x01, (unsigned char)4);
  //setBitInUInt32(&regPE_ODR, (addr >> 1) & (unsigned char)0x01, (unsigned char)2);
  //setBitInUInt32(&regPE_ODR,  addr       & (unsigned char)0x01, (unsigned char)0);
  
  //GPIOD->ODR = regPD_ODR;
  //GPIOE->ODR = regPE_ODR;
  
  /*setBitInUInt16(&regPD_BSRRMask, (unsigned int)1, (16 - ((addr >> 4) & (unsigned char)0x01) * 16) + 8);
  setBitInUInt16(&regPE_BSRRMask, (unsigned int)1, (16 - ((addr >> 3) & (unsigned char)0x01) * 16) + 5);
  setBitInUInt16(&regPE_BSRRMask, (unsigned int)1, (16 - ((addr >> 2) & (unsigned char)0x01) * 16) + 4);
  setBitInUInt16(&regPE_BSRRMask, (unsigned int)1, (16 - ((addr >> 1) & (unsigned char)0x01) * 16) + 2);
  setBitInUInt16(&regPE_BSRRMask, (unsigned int)1, (16 -  (addr       & (unsigned char)0x01) * 16) + 0);
  
  
  
  //Check bits in byte and definition of each out respectively, each bit
  if((addr & 0x01) == 0x01)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_0);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_0);
  }
  
  if((addr & 0x02) == 0x02)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_2);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_2);
  }
  
  if((addr & 0x04) == 0x04)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_4);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_4);
  }
  
  if((addr & 0x08) == 0x08)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_5);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_5);
  }
  
  if((addr & 0x10) == 0x10)
  {
    GPIO_SetBits(GPIOD, GPIO_Pin_8);
  }
  else
  {
    GPIO_ResetBits(GPIOD, GPIO_Pin_8);
  }*/
}

void resetAddressToPlis()
{
  GPIOD->BSRRH |= 0x0100;
  
  while(((GPIOD->ODR >> 8) & 0x00000001))
  ;
  
  GPIOE->BSRRH |= 0x0001;
  GPIOE->BSRRH |= 0x0004;
  GPIOE->BSRRH |= 0x0010;
  GPIOE->BSRRH |= 0x0020;
  
  /*
  unsigned long int regPD_ODR = GPIOD->ODR;
  unsigned long int regPE_ODR = GPIOE->ODR;
  
  setBitInUInt32(&regPD_ODR, (unsigned char)0, (unsigned char)8);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)5);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)4);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)2);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)0);
  
  GPIOE->ODR = regPE_ODR;
  GPIOD->ODR = regPD_ODR;
  
  
  /*GPIO_ResetBits(GPIOE, GPIO_Pin_0);
  GPIO_ResetBits(GPIOE, GPIO_Pin_2);
  GPIO_ResetBits(GPIOE, GPIO_Pin_4);
  GPIO_ResetBits(GPIOE, GPIO_Pin_5);
  GPIO_ResetBits(GPIOD, GPIO_Pin_8);*/
}

void setWriteDataToPlis(unsigned char dataToWrite)
{
  unsigned long int regPE_ODR = GPIOE->ODR;

  setBitInUInt32(&regPE_ODR,  dataToWrite       & (unsigned char)0x01, (unsigned char)8);
  setBitInUInt32(&regPE_ODR, (dataToWrite >> 1) & (unsigned char)0x01, (unsigned char)9);
  setBitInUInt32(&regPE_ODR, (dataToWrite >> 2) & (unsigned char)0x01, (unsigned char)10);
  setBitInUInt32(&regPE_ODR, (dataToWrite >> 3) & (unsigned char)0x01, (unsigned char)11);
  setBitInUInt32(&regPE_ODR, (dataToWrite >> 4) & (unsigned char)0x01, (unsigned char)12);
  setBitInUInt32(&regPE_ODR, (dataToWrite >> 5) & (unsigned char)0x01, (unsigned char)13);
  setBitInUInt32(&regPE_ODR, (dataToWrite >> 6) & (unsigned char)0x01, (unsigned char)14);
  setBitInUInt32(&regPE_ODR, (dataToWrite >> 7) & (unsigned char)0x01, (unsigned char)15);
  
  GPIOE->ODR = regPE_ODR;
  
  /*
  //Check bits in byte and definition of each out respectively, each bit
  if((dataToWrite & 0x01) == 0x01)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_8);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_8);
  }
  
  if((dataToWrite & 0x02) == 0x02)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_9);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_9);
  }
  
  if((dataToWrite & 0x04) == 0x04)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_10);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_10);
  }
  
  if((dataToWrite & 0x08) == 0x08)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_11);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_11);
  }
  
  if((dataToWrite & 0x10) == 0x10)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_12);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_12);
  }
  
  if((dataToWrite & 0x20) == 0x20)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_13);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_13);
  }
  
  if((dataToWrite & 0x40) == 0x40)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_14);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_14);
  }
  
  if((dataToWrite & 0x80) == 0x80)
  {
    GPIO_SetBits(GPIOE, GPIO_Pin_15);
  }
  else
  {
    GPIO_ResetBits(GPIOE, GPIO_Pin_15);
  }*/
}

void resetWriteDataToPlis()
{
  unsigned long int regPE_ODR = GPIOE->ODR;

  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)8);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)9);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)10);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)11);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)12);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)13);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)14);
  setBitInUInt32(&regPE_ODR, (unsigned char)0, (unsigned char)15);
  
  GPIOE->ODR = regPE_ODR;
  
  /*GPIO_ResetBits(GPIOE, GPIO_Pin_8);
  GPIO_ResetBits(GPIOE, GPIO_Pin_9);
  GPIO_ResetBits(GPIOE, GPIO_Pin_10);
  GPIO_ResetBits(GPIOE, GPIO_Pin_11);
  GPIO_ResetBits(GPIOE, GPIO_Pin_12);
  GPIO_ResetBits(GPIOE, GPIO_Pin_13);
  GPIO_ResetBits(GPIOE, GPIO_Pin_14);
  GPIO_ResetBits(GPIOE, GPIO_Pin_15);*/
}

unsigned char readDataByteFromPlis()
{
  unsigned long int regPD_IDR = GPIOD->IDR;
  unsigned long int regPE_IDR = GPIOE->IDR;
  unsigned char dataByte = 0x00;

  setBitInUChar(&dataByte, (unsigned char)(regPD_IDR & 0x00000001), (unsigned char)0);
  setBitInUChar(&dataByte, (unsigned char)((regPD_IDR >> 1) & 0x00000001), (unsigned char)1);
  setBitInUChar(&dataByte, (unsigned char)((regPD_IDR >> 2) & 0x00000001), (unsigned char)2);
  setBitInUChar(&dataByte, (unsigned char)((regPD_IDR >> 3) & 0x00000001), (unsigned char)3);
  setBitInUChar(&dataByte, (unsigned char)((regPD_IDR >> 4) & 0x00000001), (unsigned char)4);
  setBitInUChar(&dataByte, (unsigned char)((regPD_IDR >> 6) & 0x00000001), (unsigned char)5);
  setBitInUChar(&dataByte, (unsigned char)((regPD_IDR >> 7) & 0x00000001), (unsigned char)6);
  setBitInUChar(&dataByte, (unsigned char)((regPE_IDR >> 1) & 0x00000001), (unsigned char)7);
  
  return dataByte;
  
  /*unsigned char byte = 0x00;
  
  if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_0))
  {
    byte |= 0x01;
  }

  if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1))
  {
    byte |= 0x02;
  }
  
  if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2))
  {
    byte |= 0x04;
  }
  
  if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3))
  {
    byte |= 0x08;
  }
  
  if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4))
  {
    byte |= 0x10;
  }
  
  if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6))
  {
    byte |= 0x20;
  }
  
  if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7))
  {
    byte |= 0x40;
  }
  
  if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1))
  {
    byte |= 0x80;
  }
  
  return byte;*/
}

//-------------------------------------Block of functions to exchange with PLIS-----------------------------------------------------
//Write data to address to PLIS
void writeDataOnAddrPlis(unsigned char addr, unsigned char dataByte)
{
  setWriteDataToPlis(dataByte);
  setAddressToPlis(addr);
  resetAddressToPlis();
  resetWriteDataToPlis();
}

unsigned char getDataByteFromPlis(unsigned char address)
{
  //receiving data from PLIS
  //write channel number for reading on data read address
  writeDataOnAddrPlis((unsigned char)0x10, address);
  
  //read byte from data bus
  return readDataByteFromPlis();
}

unsigned char getBitFromDataByteFromPlis(unsigned char address, unsigned char numberOfBit)
{
  return (unsigned char)((getDataByteFromPlis(address) >> numberOfBit) & 0x01);
}

//------Bit set or reset and get functions------
/*Set one bit in unsigned int32
@params:
  pWord32 - word32 for bit set(reference value)
  bitValue - value to set(0 or 1)
  bitNumber - bit number to set
@retval: None
*/
void setBitInUInt32(unsigned long int *pWord32, unsigned char bitValue, unsigned char bitNumber)
{
  if(((bitValue != 1) && (bitValue != 0)) || ((bitNumber >= sizeof(unsigned long int)*8) || (bitNumber < 0)))
  {
    return;
  }
  
  if(bitValue == (unsigned char)0x01)
  {
    *pWord32 |= (unsigned long int)0x00000001 << bitNumber;
  }
  else if(bitValue == (unsigned char)0x00)
  {
    *pWord32 &= ( ((unsigned long int)0xfffffffe << bitNumber) | ((unsigned long int)0xffffffff >> (sizeof(unsigned long int) * 8 - bitNumber)) );
  }
}

/*Set one bit in unsigned int16
@params:
  pWord16 - word16 for bit set(reference value)
  bitValue - value to set(0 or 1)
  bitNumber - bit number to set
@retval: None
*/
void setBitInUInt16(unsigned int *pWord16, unsigned char bitValue, unsigned char bitNumber)
{
  if(((bitValue != 1) && (bitValue != 0)) || ((bitNumber >= sizeof(unsigned int)*8) || (bitNumber < 0)))
  {
    return;
  }
  
  if(bitValue == (unsigned char)0x01)
  {
    *pWord16 |= (unsigned int)0x0001 << bitNumber;
  }
  else if(bitValue == (unsigned char)0x00)
  {
    *pWord16 &= ( ((unsigned int)0xfffe << bitNumber) | ((unsigned int)0xffff >> (sizeof(unsigned int) * 8 -  bitNumber)) );
  }
}

/*Set one bit in unsigned char
@params:
  pWord16 - uchar for bit set(reference value)
  bitValue - value to set(0 or 1)
  bitNumber - bit number to set
@retval: None
*/
void setBitInUChar(unsigned char *pUChar, unsigned char bitValue, unsigned char bitNumber)
{
  if(((bitValue != 1) && (bitValue != 0)) || ((bitNumber >= sizeof(unsigned char)*8) || (bitNumber < 0)))
  {
    return;
  }
  
  if(bitValue == (unsigned char)0x01)
  {
    *pUChar |= (unsigned char)0x01 << bitNumber;
  }
  else if(bitValue == (unsigned char)0x00)
  {
    *pUChar &= ( ((unsigned char)0xfe << bitNumber) | ((unsigned char)0xff >> (sizeof(unsigned char) * 8 -  bitNumber)) );
  }
}

/*Set one bit in byte
@params:
  byte - byte for bit set(reference value)
  bitNumber - bit number to set
@retval: None
*/
void setBitInByte(unsigned char *pByte, unsigned char bitNumber)
{
  if(bitNumber == 0)
  {
    *pByte |= (unsigned char) 0x01;
  }
  else if(bitNumber == 1)
  {
    *pByte |= (unsigned char) 0x02;
  }
  else if(bitNumber == 2)
  {
    *pByte |= (unsigned char) 0x04;
  }
  else if(bitNumber == 3)
  {
    *pByte |= (unsigned char) 0x08;
  }
  else if(bitNumber == 4)
  {
    *pByte |= (unsigned char) 0x10;
  }
  else if(bitNumber == 5)
  {
    *pByte |= (unsigned char) 0x20;
  }
  else if(bitNumber == 6)
  {
    *pByte |= (unsigned char) 0x40;
  }
  else if(bitNumber == 7)
  {
    *pByte |= (unsigned char) 0x80;
  }
}

/*Reset one bit in byte
@params:
  byte - byte for bit reset(reference value)
  bitNumber - bit number to reset
@retval: None
*/
void resetBitInByte(unsigned char *pByte, unsigned char bitNumber)
{
  if(bitNumber == 0)
  {
    *pByte &= (unsigned char) 0xfe;
  }
  else if(bitNumber == 1)
  {
    *pByte &= (unsigned char) 0xfd;
  }
  else if(bitNumber == 2)
  {
    *pByte &= (unsigned char) 0xfb;
  }
  else if(bitNumber == 3)
  {
    *pByte &= (unsigned char) 0xf7;
  }
  else if(bitNumber == 4)
  {
    *pByte &= (unsigned char) 0xef;
  }
  else if(bitNumber == 5)
  {
    *pByte &= (unsigned char) 0xdf;
  }
  else if(bitNumber == 6)
  {
    *pByte &= (unsigned char) 0xbf;
  }
  else if(bitNumber == 7)
  {
    *pByte &= (unsigned char) 0x7f;
  }
}

/*Get one bit from byte
@params:
  byte - byte to bit extracting
  bitNumber - number of extracted bit
@retval: requested bit value
*/
unsigned char getBitFromByte(unsigned char byte, unsigned char numberOfBit)
{
  return (byte >> numberOfBit) & 0x01;
}


//------Functions to control of coordinate detectors------
/*Permission to reset detector on the hit a limit switch
@params:
permissoin - byte of permission reset coordinate detector on the hit a limit switch
  1 - alowed
  0 - forbidden
@retval: None
*/
void permResetZCoordDetect(unsigned char detectorNumb, unsigned char permission)
{
  if(permission == 1)
  {
    setBitInByte(&coordDetect.regOfCtrlCoordDetect[detectorNumb], 2);
  }
  else if(permission == 0)
  {
    resetBitInByte(&coordDetect.regOfCtrlCoordDetect[detectorNumb], 2);
  }
  
  writeDataOnAddrPlis((unsigned char)0x02, coordDetect.regOfCtrlCoordDetect[detectorNumb]);
}

void permResetZCurCoordDetect(unsigned char permission)
{
   if(permission == 1)
  {
    setBitInByte(&coordDetect.regOfCtrlCoordDetect[coordDetect.active], 2);
  }
  else if(permission == 0)
  {
    resetBitInByte(&coordDetect.regOfCtrlCoordDetect[coordDetect.active], 2);
  }
  
  writeDataOnAddrPlis((unsigned char)0x02, coordDetect.regOfCtrlCoordDetect[coordDetect.active]);
}

void permResetZCoordDetectColona(unsigned char permission)
{
  permResetZCoordDetect(0, permission);
}

void permResetZCoordDetectKaretka(unsigned char permission)
{
  permResetZCoordDetect(1, permission);
}

void permResetZCoordDetectAzimut(unsigned char permission)
{
  permResetZCoordDetect(2 , permission);
}


/*Reset coordinate detector 
@params: None
@retval: None
*/
void resetCoordDetect(unsigned char detectorNumb)
{
  setBitInByte(&coordDetect.regOfCtrlCoordDetect[detectorNumb], 3);
    
  writeDataOnAddrPlis((unsigned char)0x02, coordDetect.regOfCtrlCoordDetect[detectorNumb]);

  resetBitInByte(&coordDetect.regOfCtrlCoordDetect[detectorNumb], 3);
}

void resetCurCoordDetect()
{
  setBitInByte(&coordDetect.regOfCtrlCoordDetect[coordDetect.active], 3);
    
  writeDataOnAddrPlis((unsigned char)0x02, coordDetect.regOfCtrlCoordDetect[coordDetect.active]);

  resetBitInByte(&coordDetect.regOfCtrlCoordDetect[coordDetect.active], 3);
}

void resetCoordDetectColona()
{
  resetCoordDetect(0);
}

void resetCoordDetectKaretka()
{
  resetCoordDetect(1);
}

void resetCoordDetectAzimut()
{
  resetCoordDetect(2);
}


/*Set current coordinate detector 
@params:
detectorNumb - number of coordinate detector
  0x00 - kolona
  0x01 - karetka
  0x02 - azimut
permission - permission to reset on zero limit switch
  1 - allow
  0 - denied
reset - byte of reset
  1 - reset
  0 - not reset
@retval: None
*/
void setCurCoordDet(unsigned char detectorNumb, unsigned char permission, unsigned char reset)
{
  coordDetect.regOfCtrlCoordDetect[detectorNumb] = (unsigned char)0x00 | detectorNumb;
  coordDetect.active = detectorNumb;
  
  if(permission == 1)
  {
    setBitInByte(&coordDetect.regOfCtrlCoordDetect[detectorNumb], 2);
  }
  else if(permission == 0)
  {
    resetBitInByte(&coordDetect.regOfCtrlCoordDetect[detectorNumb], 2);
  }
  
  if(reset == 1)
  {
    setBitInByte(&coordDetect.regOfCtrlCoordDetect[detectorNumb], 3);
  }
  
  writeDataOnAddrPlis((unsigned char)0x02, coordDetect.regOfCtrlCoordDetect[detectorNumb]);
  
  if(reset == 1)
  {
    resetBitInByte(&coordDetect.regOfCtrlCoordDetect[detectorNumb], 3);
  }
}

void setCurCoordDetColona(unsigned char permission, unsigned char reset)
{
  setCurCoordDet((unsigned char) 0x00, permission, reset);
}

void setCurCoordDetKaretka(unsigned char permission, unsigned char reset)
{
  setCurCoordDet((unsigned char) 0x01, permission, reset);
}

void setCurCoordDetAzimut(unsigned char permission, unsigned char reset)
{
  setCurCoordDet((unsigned char) 0x02, permission, reset);
}

/*Set current coordinate detector 
@params:
detectorNumb - number of coordinate detector
  0x00 - kolona
  0x01 - karetka
  0x02 - azimut
@retval: None
*/
void setCoordDet(unsigned char detectorNumb)
{
  coordDetect.regOfCtrlCoordDetect[detectorNumb] = (unsigned char)0x00 | detectorNumb;
  coordDetect.active = detectorNumb;
  
  writeDataOnAddrPlis((unsigned char)0x02, coordDetect.regOfCtrlCoordDetect[detectorNumb]);
}

void setCoordDetColona()
{
  setCoordDet((unsigned char) 0x00);
}

void setCoordDetKaretka()
{
  setCoordDet((unsigned char) 0x01);
}

void setCoordDetAzimut()
{
  setCoordDet((unsigned char) 0x02);
}

/*Get current(active) coordinate detector number
@params: None
@retval the number of current coordinate detector
*/
unsigned char getActiveCoordDetect()
{
  return coordDetect.active;
}

/*Get permisson to zeroing of coordinate detector
@params:
detectorNumb - number of detector bit value which we watn to get
@retval: permission to zeroing
  1 - granted
  0 - denied
*/
unsigned char getPermResCurCoordDet(unsigned char detectorNumb)
{
  return getBitFromByte(coordDetect.regOfCtrlCoordDetect[detectorNumb], (unsigned char)2);
}


//------Functions to control of PLIS Comparators------
/*Set compairing direction
@params:
direction - direction of motion of coordinate detector
  0 - motion to increase coordinate
  1 - motion to decrease coordinate
@retval: None
*/
void dirOfADCComp(unsigned char direction)
{
  if(direction == 1)
  {
    setBitInByte(&regOfCtrlCompar, 0);
  }
  else if(direction == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 0);
  }
  
  writeDataOnAddrPlis((unsigned char)0x03, regOfCtrlCompar);
}

void dirOfMotorComp(unsigned char direction)
{
  if(direction == 1)
  {
    setBitInByte(&regOfCtrlCompar, 3);
  }
  else if(direction == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 3); 
  }
  
  writeDataOnAddrPlis((unsigned char)0x03, regOfCtrlCompar);
}

void dirOfSlowSpeedComp(unsigned char direction)
{
  if(direction == 1)
  {
    setBitInByte(&regOfCtrlCompar, 5);
  }
  else if(direction == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 5);
  }
  
  writeDataOnAddrPlis((unsigned char)0x03, regOfCtrlCompar);
}

/*Set pulse granting
@params:
permission - bit of permission pulse granting
  0 - denied
  1 - granted
@retval: None
*/
void permOfADCComp(unsigned char permission)
{
  if(permission == 1)
  {
    setBitInByte(&regOfCtrlCompar, 1);
  }
  else if(permission == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 1);
  }
  
  writeDataOnAddrPlis((unsigned char)0x03, regOfCtrlCompar);
}

void permOfMotorComp(unsigned char permission)
{
  if(permission == 1)
  {
    setBitInByte(&regOfCtrlCompar, 4);
  }
  else if(permission == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 4);
  }
  
  writeDataOnAddrPlis((unsigned char)0x03, regOfCtrlCompar);
}

void permOfSlowSpeedComp(unsigned char permission)
{
  if(permission == 1)
  {
    setBitInByte(&regOfCtrlCompar, 6);
  }
  else if(permission == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 6);
  }
  
  writeDataOnAddrPlis((unsigned char)0x03, regOfCtrlCompar);
}

/*Set register of controlling detector`s comparators
@params:
dir1 - direction comparsion of the comparator ADC
  0 - more or equal
  1 - less or equal
dir2 - direction comparsion of the motor stop comparator
  0 - more or equal
  1 - less or equal
dir3 - direction comparsion of the slow speed comparator
  0 - more or equal
  1 - less or equal
per1 - permission pulse granting on ADC comparator triggering
  0 - denied
  1 - granted
per2 - permission pulse granting on motor stop comparator triggering
  0 - denied
  1 - granted
per3 - permission pulse granting on slow speed comparator triggering
  0 - denied
  1 - granted
@retval: None
*/
void setComparatorsOptions(unsigned char dir1, unsigned char per1, unsigned char dir2, unsigned char per2, unsigned char dir3, unsigned char per3)
{
  if(dir1 == 1)
  {
    setBitInByte(&regOfCtrlCompar, 0);
  }
  else if(dir1 == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 0);
  }
  
  if(per1 == 1)
  {
    setBitInByte(&regOfCtrlCompar, 1);
  }
  else if(per1 == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 1);
  }
  
  if(dir2 == 1)
  {
    setBitInByte(&regOfCtrlCompar, 3);
  }
  else if(dir2 == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 3);
  }
  
  if(per2 == 1)
  {
    setBitInByte(&regOfCtrlCompar, 4);
  }
  else if(per2 == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 4);
  }
  
  if(dir3 == 1)
  {
    setBitInByte(&regOfCtrlCompar, 5);
  }
  else if(dir3 == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 5);
  }
  
  if(per3 == 1)
  {
    setBitInByte(&regOfCtrlCompar, 6);
  }
  else if(per3 == 0)
  {
    resetBitInByte(&regOfCtrlCompar, 6);
  }
  
  writeDataOnAddrPlis((unsigned char)0x03, regOfCtrlCompar);
}

/*Get direction of ADC Compar of Current Coordinate Detector
@params: None
@retval: comparator direction
  0 - to increase corrdinates
  1 - to decrease corrdinates
*/
unsigned char getDirOfADCComp()
{
  return getBitFromByte(regOfCtrlCompar, (unsigned char)0);
}

/*Get direction of Motor Compar of Current Coordinate Detector
@params: None
@retval: comparator direction
  0 - to increase corrdinates
  1 - to decrease corrdinates
*/
unsigned char getDirOfMotorComp()
{
  return getBitFromByte(regOfCtrlCompar, (unsigned char)3);
}

/*Get direction of Motor Compar of Current Coordinate Detector
@params: None
@retval: comparator direction
  0 - to increase corrdinates
  1 - to decrease corrdinates
*/
unsigned char getDirOfSlowSpeedComp()
{
  return getBitFromByte(regOfCtrlCompar, (unsigned char)5);
}

/*Get pulse premisson of ADC Comparator
@params: None
@retval: comparator direction
  0 - denied
  1 - granted
*/
unsigned char getPermOfADCComp()
{
  return getBitFromByte(regOfCtrlCompar, (unsigned char)1);
}

/*Get pulse premisson of Motor Comparator
@params: None
@retval: comparator direction
  0 - denied
  1 - granted
*/
unsigned char getPermOfMotorComp()
{
  return getBitFromByte(regOfCtrlCompar, (unsigned char)4);
}

/*Get pulse premisson of Slow Speed Comparator
@params: None
@retval: comparator direction
  0 - denied
  1 - granted
*/
unsigned char getPermOfSlowSpeedComp()
{
  return getBitFromByte(regOfCtrlCompar, (unsigned char)6);
}


//------Functions to writing the coordinates for comparators------
/*Write coordinate for comparator ADC
@params:
  coord - coordinate for writing
@retval: None
*/
void writeCoordToADCComp(long int coord)
{
  for(unsigned char i = (unsigned char)0x04; i < (unsigned char)0x07; i++)
  {
    writeDataOnAddrPlis(i, (unsigned char)((coord >> ( (i - 0x04) * 8) ) & 0x000000ff));
  }
  
  
}

void writeCoordToMotorComp(long int coord)
{
  for(unsigned char i = (unsigned char)0x07; i < (unsigned char)0x0A; i++)
  {
    writeDataOnAddrPlis(i, (unsigned char)((coord >> ( (i - 0x07) * 8) ) & 0x000000ff));
  }
}

void writeCoordToSlowSpeedComp(long int coord)
{
  for(unsigned char i = (unsigned char)0x0A; i < (unsigned char)0x0D; i++)
  {
    writeDataOnAddrPlis(i, (unsigned char)((coord >> ( (i - 0x0A) * 8) ) & 0x000000ff));
  }
}


//------Functions for motors controlling------
/*Get engine number from detector number
@params:
detectNumb - number of detector
  0 - Kolona
  1 - Karetka
  2 - Azimut
@retval: number of engine
  5 - Kolona
  6 - Karetka
  2 - Azimut
  255 - wrong detector number
*/
unsigned char getEngNumbFromDetNumb(unsigned char detectNumb)
{
  if(detectNumb == (unsigned char)0x00)
  {
    return 5;
  }
  else if(detectNumb == (unsigned char)0x01)
  {
    return 6;
  }
  else if(detectNumb == (unsigned char)0x02)
  {
    return 2;
  }
  else
  {
    return 255;
  }
}

/*Get detector number from engine number
@params:
detectNumb - number of engine
  5 - Kolona
  6 - Karetka
  2 - Azimut
@retval: number of detector
  0 - Kolona
  1 - Karetka
  2 - Azimut
  255 - wrong engine number
*/
unsigned char getDetNumbFromEngNumb(unsigned char engineNumb)
{
  if(engineNumb == (unsigned char)0x05)
  {
    return 0;
  }
  else if(engineNumb == (unsigned char)0x06)
  {
    return 1;
  }
  else if(engineNumb == (unsigned char)0x02)
  {
    return 2;
  }
  else
  {
    return 255;
  }
}

/*Stop Engines
@params: None
@retval: None
*/
void stopEngines()
{
  regOfSwitchEngines = (unsigned char)0x00;
  writeDataOnAddrPlis((unsigned char)0x0e, (unsigned char)0x00);
  writeDataOnAddrPlis((unsigned char)0x0d, regOfSwitchEngines);
}

/*Switch the engine
@params:
  engineNumb - number of engine(number of engine`s bit). Other engines will be turn off
@retval: None
*/
void switchEngine(unsigned char engineNumb)
{
  if(((regOfSwitchEngines >> engineNumb) & (unsigned char)0x01) == (unsigned char)0x01)
  {
    return;
  }
  
  if((engineNumb < 0) || (engineNumb > 6))
  {
    return;
  }
  
  motionDirectOfEngine((signed char)0);
  slowSpeedSwitch((unsigned char)0);
  switchEngineSpeed((unsigned char)0x00);
  switchBrakingMode((unsigned char)1);
  
  stopEngines();
  setBitInByte(&regOfSwitchEngines, engineNumb);
  writeDataOnAddrPlis((unsigned char)0x0d, regOfSwitchEngines);
}

/*Get active engine
@params: None
@retval: number of active engines
  0 - Ugol mesta
  1 - Peremescheniye
  2 - Azimut
  3 - Fokal
  4 - Podyom
  5 - Kolona
  6 - Karetka
  255 - All engines are disabled
*/
unsigned char getActiveEngine()
{
  unsigned char active = (unsigned char)0xff;
  
  for(unsigned char i = 0; i < 8; i++)
  {
    if(((regOfSwitchEngines >> i) & (unsigned char)0x01) == (unsigned char)0x01)
    {
      active = i;
      break;
    }
  }
  
  return active;
}

/*Switch the engine on/off
@params: None
@retval: None
*/
void switchEngineUgolMesta()
{
  switchEngine(0);
}

void switchEnginePeremesch()
{
  switchEngine(1);
}

void switchEngineAzimut()
{
  switchEngine(2);
}

void switchEngineFokal()
{
  switchEngine(3);
}

void switchEnginePodyom()
{
  switchEngine(4);
}

void switchEngineKolona()
{
  switchEngine(5);
}

void switchEngineKaretka()
{
  switchEngine(6);
}


//------Functions for controlling motor control unit------
/*Switch motion direction of motor
@params:
  direction - motor motion direction
    1 - motion to increase coordinates(right/bottom)
    -1 - motion to decrease coordinates(left/top)
    0 - engines are not in motion
@retval: None
If no one of engines are changed function terminated without exchanging with PLIS
P.S. Very Important!!! In the first time we reset bit of active engine`s direction, only them choosing new engine`s direction, otherwise it woud be a mistake
*/
void motionDirectOfEngine(signed char direction)
{
  if(getActiveEngine() == 0xff)
  {
    return;
  }
  
  if(direction == 1)
  {
    resetBitInByte(&regOfCtrlEngines[getActiveEngine()], 0);
    setBitInByte(&regOfCtrlEngines[getActiveEngine()], 1);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
  else if(direction == -1)
  {
    resetBitInByte(&regOfCtrlEngines[getActiveEngine()], 1);
    setBitInByte(&regOfCtrlEngines[getActiveEngine()], 0);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
  else if(direction == 0)
  {
    resetBitInByte(&regOfCtrlEngines[getActiveEngine()], 0);
    resetBitInByte(&regOfCtrlEngines[getActiveEngine()], 1);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
}

/*Turning on/off slow speed
@params:
  toggle - slow speed toggle
    1 - turn on slow speed
    0 - turn off slow speed
@retval: None
If no one of engines are changed function terminated without exchanging with PLIS
*/
void slowSpeedSwitch(unsigned char toggle)
{
  if(getActiveEngine() == 0xff)
  {
    return;
  }
  
  if(toggle == 1)
  {
    setBitInByte(&regOfCtrlEngines[getActiveEngine()], 2);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
  else if(toggle == 0)
  {
    resetBitInByte(&regOfCtrlEngines[getActiveEngine()], 2);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
}

/*Switch engine speed
@params:
  speedCode - code of the speed
    0x00 - 100% speed
    0x01 - 75% speed
    0x10 - 50% speed
    0x11 - 25% speed
@retval: None
If no one of engines are changed function terminated without exchanging with PLIS
*/
void switchEngineSpeed(unsigned char speedCode)
{
  if(getActiveEngine() == 0xff)
  {
    return;
  }
  
  if((speedCode & (unsigned char)0x0f) == (unsigned char)0x01)
  {
    setBitInByte(&regOfCtrlEngines[getActiveEngine()], 5);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
  else if((speedCode & (unsigned char)0x0f) == (unsigned char)0x00)
  {
    resetBitInByte(&regOfCtrlEngines[getActiveEngine()], 5);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
  
  if(((speedCode >> 4) & (unsigned char)0x0f) == (unsigned char)0x01)
  {
    setBitInByte(&regOfCtrlEngines[getActiveEngine()], 6);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
  else if(((speedCode >> 4) & (unsigned char)0x0f) == (unsigned char)0x00)
  {
    resetBitInByte(&regOfCtrlEngines[getActiveEngine()], 6);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
  
}

/*Switch braking mode
@params:
  brakingMode - code of the speed
    1 - standard mode of braking
    0 - additional mode of braking
@retval: None
If no one of engines are changed function terminated without exchanging with PLIS
*/
void switchBrakingMode(unsigned char brakingMode)
{
  if(getActiveEngine() == 0xff)
  {
    return;
  }
  
  if(brakingMode == 1)
  {
    setBitInByte(&regOfCtrlEngines[getActiveEngine()], 3);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
  else if(brakingMode == 0)
  {
    resetBitInByte(&regOfCtrlEngines[getActiveEngine()], 3);
    writeDataOnAddrPlis((unsigned char)0x0e, regOfCtrlEngines[getActiveEngine()]);
  }
}

/*Get active engine
@params: None
@retval: number of active engines
  0 - additional mode
  1 - standard mode
  255 - ERROR no one of engines are changed
P.S. when "column" motor connected, Braking modee must be chosen to 1(standard mode)
*/
unsigned char getBrakingMode()
{
  if(getActiveEngine() == 0xff)
  {
    return 0xff;
  }
  
  return getBitFromByte(regOfCtrlEngines[getActiveEngine()], (unsigned char)3);
}

/*Get active engine speed
@params: None
@retval: number of active engines
  0x00 - 100%
  0x01 - 75%
  0x10 - 50%
  0x11 - 25%
  0xff - ERROR no one of engines are changed
*/
unsigned char getEngineSpeed()
{
  if(getActiveEngine() == 0xff)
  {
    return 0xff;
  }
  
  return getBitFromByte(regOfCtrlEngines[getActiveEngine()], (unsigned char)5) | ((getBitFromByte(regOfCtrlEngines[getActiveEngine()], (unsigned char)6) << 4) & 0x10);
}

//------Functions for controlling switches VV88M------
/*Switch VV88M to working from DKS
@params:
  switchNumb - number of switch 1..8
@retval: None
*/
void setVV88MFromDKS(unsigned char switchNumb)
{
  resetBitInByte(&regOfVV88MStates, switchNumb - 1);
  writeDataOnAddrPlis((unsigned char)0x0f, regOfVV88MStates);
}

/*Switch VV88M to working from Zond
@params:
  switchNumb - number of switch 1..8
@retval: None
*/
void setVV88MFromZond(unsigned char switchNumb)
{
  setBitInByte(&regOfVV88MStates, switchNumb - 1);
  writeDataOnAddrPlis((unsigned char)0x0f, regOfVV88MStates);
}

/*Get VV88 working mode
@params:
  switchNumb - number of switch 1..8
@retval: working mode
  0 - working from DKS
  1 - working from Zond
*/
unsigned char getSwitchWorkMode(unsigned char switchNumb)
{
  return getBitFromByte(regOfVV88MStates, switchNumb - 1);
}


//------Functions for reading special datas from PLIS------
/*Get current detector coordinate
@params: None
@retval: current detector coordinate (long int 32 bit)
*/
long int getCurrCoordFromDetect()
{
  long int coord = (long int)0x00000000;
  
  coord |= (long int)getDataByteFromPlis((unsigned char)0x01);
  coord |= (((long int)getDataByteFromPlis((unsigned char)0x02) << 8) & 0x0000ff00);
  coord |= (((long int)getDataByteFromPlis((unsigned char)0x03) << 16) & 0x00ff0000);
  
  //converting coordinate to 32-bit type
  if((coord & 0x00800000) != 0x00000000)
  {
    //if sign(23) bit equal 1 convert the highest byte to ff
    coord |= 0xff000000;
  }
  
  return coord;
}

/*Bit of requesting new coordinate for ADC comparator if last coordinate was handled
@params: None
@retval: Bit of requesting new coordinate for ADC comparator
  1 - request is present
  0 - no request
*/
unsigned char getStateBitADCReq()
{
  return getBitFromDataByteFromPlis((unsigned char)0x00, (unsigned char)0);
}

/*Bit of requesting new coordinate for stop comparator if last coordinate was handled
@params: None
@retval: Bit of requesting new coordinate for stop comparator
  1 - request is present
  0 - no request
*/
unsigned char getStateBitStopReq()
{
  return getBitFromDataByteFromPlis((unsigned char)0x00, (unsigned char)1);
}

/*Bit of requesting new coordinate for SlowSpeed comparator if last coordinate was handled
@params: None
@retval: Bit of requesting new coordinate for slowSpeed comparator
  1 - request is present
  0 - no request
*/
unsigned char getStateBitSlowReq()
{
  return getBitFromDataByteFromPlis((unsigned char)0x00, (unsigned char)2);
}

/*Bit of requesting high bit of coordinate detector number
@params: None
@retval: High bit of coordinate detector number
  1 
  0 
*/
unsigned char getStateBitVd0()
{
  return getBitFromDataByteFromPlis((unsigned char)0x00, (unsigned char)3);
}

/*Bit of requesting low bit of coordinate detector number
@params: None
@retval: Low bit of coordinate detector number
  1 
  0 
*/
unsigned char getStateBitVd1()
{
  return getBitFromDataByteFromPlis((unsigned char)0x00, (unsigned char)4);
}

/*Bit of permission reset coordinate detector on the hit a limit switch 
@params: None
@retval: Bit of permission reset coordinate detector on the hit a limit switch 
  1 - allowed
  0 - forbidden
*/
unsigned char getStateBitPermZeroReset()
{
  return getBitFromDataByteFromPlis((unsigned char)0x00, (unsigned char)5);
}

/*Bit which indicating commands receive form PC or from console
@params: None
@retval: Indicating bit
  1 - form PC
  0 - from console
*/
unsigned char getStateBitEVMOrMest()
{
  return getBitFromDataByteFromPlis((unsigned char)0x04, (unsigned char)0);
}

/*Bit which indicating ATV ready
@params: None
@retval: Indicating bit
  1 - ATV is ready
  0 - ATV is not ready
*/
unsigned char getStateBitATVReady()
{
  return getBitFromDataByteFromPlis((unsigned char)0x04, (unsigned char)1);
}

/*Bit which indicating ATV crash
@params: None
@retval: Indicating bit
  1 - ATV is crashed
  0 - ATV is not crashed
*/
unsigned char getStateBitATVCrash()
{
  return getBitFromDataByteFromPlis((unsigned char)0x04, (unsigned char)2);
}

/*Bit which indicating ATV blocking
@params: None
@retval: Indicating bit
  1 - ATV is blocked
  0 - ATV is not blocked
*/
unsigned char getStateBitATVBlocking()
{
  return getBitFromDataByteFromPlis((unsigned char)0x04, (unsigned char)3);
}

/*Bit which indicating hitting on right limited switch
@params: None
@retval: Indicating bit
  1 - limited switch was hit
  0 - limited switch wasn`t hit
*/
unsigned char getStateBitStopR()
{
  return getBitFromDataByteFromPlis((unsigned char)0x04, (unsigned char)4);
}

/*Bit which indicating hitting on left limited switch(zero switch)s
@params: None
@retval: Indicating bit
  1 - limited switch was hit
  0 - limited switch wasn`t hit
*/
unsigned char getStateBitStopL()
{
  return getBitFromDataByteFromPlis((unsigned char)0x04, (unsigned char)5);
}

/*Bit which indicating motion to increase coordinates
@params: None
@retval: Indicating bit
  1 - motion to increase coordinates
  0 - no motion to increase coordinates
*/
unsigned char getStateBitDriveR()
{
  return getBitFromDataByteFromPlis((unsigned char)0x04, (unsigned char)6);
}

/*Bit which indicating motion to dencrease coordinates
@params: None
@retval: Indicating bit
  1 - motion to dencrease coordinates
  0 - no motion to dencrease coordinates
*/
unsigned char getStateBitDriveL()
{
  return getBitFromDataByteFromPlis((unsigned char)0x04, (unsigned char)7);
}

/*Bit which indicating working motor on slow speed
@params: None
@retval: Indicating bit
  1 - motion working on slow speed
  0 - motion not working on slow speed
*/
unsigned char getStateBitSlowSpeed()
{
  return getBitFromDataByteFromPlis((unsigned char)0x05, (unsigned char)0);
}

/*Bit which indicating the motor ugol mesta is connected
@params: None
@retval: Indicating bit
  1 - motor ugol mesta is connected
  0 - motor ugol mesta is not connected
*/
unsigned char getStateBitEngUgolMesta()
{
  return getBitFromDataByteFromPlis((unsigned char)0x05, (unsigned char)1);
}

/*Bit which indicating the motor peremescheniye is connected
@params: None
@retval: Indicating bit
  1 - motor peremescheniye is connected
  0 - motor peremescheniye is not connected
*/
unsigned char getStateBitEngPeremesch()
{
  return getBitFromDataByteFromPlis((unsigned char)0x05, (unsigned char)2);
}

/*Bit which indicating the motor azimut is connected
@params: None
@retval: Indicating bit
  1 - motor azimut is connected
  0 - motor azimut is not connected
*/
unsigned char getStateBitEngAzimut()
{
  return getBitFromDataByteFromPlis((unsigned char)0x05, (unsigned char)3);
}

/*Bit which indicating the motor fokal is connected
@params: None
@retval: Indicating bit  
  1 - motor fokal is connected
  0 - motor fokal is not connected
*/
unsigned char getStateBitEngFokal()
{
  return getBitFromDataByteFromPlis((unsigned char)0x05, (unsigned char)4);
}

/*Bit which indicating the motor podyom is connected
@params: None
@retval: Indicating bit
  1 - motor podyom is connected
  0 - motor podyom is not connected
*/
unsigned char getStateBitEngPodyom()
{
  return getBitFromDataByteFromPlis((unsigned char)0x05, (unsigned char)5);
}

/*Bit which indicating the motor colona is connected
@params: None
@retval: Indicating bit
  1 - motor colona is connected
  0 - motor colona is not connected
*/
unsigned char getStateEngColona()
{
  return getBitFromDataByteFromPlis((unsigned char)0x05, (unsigned char)6);
}

/*Bit which indicating the motor karetka is connected
@params: None
@retval: Indicating bit
  1 - motor karetka is connected
  0 - motor karetka is not connected
*/
unsigned char getStateEngKaretka()
{
  return getBitFromDataByteFromPlis((unsigned char)0x05, (unsigned char)7);
}

/*Active engine which received from PLIS
@params: None
@retval: Active engine number
  0 - engine ugol mesta
  1 - engine peremesch
  2 - engine azimut
  3 - engine fokal
  4 - engine podyom
  5 - engine kolona
  6 - engine karetka
  255 - no one engine is connected
*/
unsigned char getActiveEngineFromPLIS()
{
  if(getStateBitEngUgolMesta() == 1)
  {
    return 0;
  }
  else if(getStateBitEngPeremesch() == 1)
  {
    return 1;
  }
  else if(getStateBitEngAzimut() == 1)
  {
    return 2;
  }
  else if(getStateBitEngFokal() == 1)
  {
    return 3;
  }
  else if(getStateBitEngPodyom() == 1)
  {
    return 4;
  }
  else if(getStateEngColona() == 1)
  {
    return 5;
  }
  else if(getStateEngKaretka() == 1)
  {
    return 6;
  }
  else
  {
    return 255;
  }
}


//------Functoin to check connection with PLIS------
/*Checking connection with PLIS
@params:
  data - data which must transmit to PLIS and receive there
@retval: byte from the PLIS (must match with transmited byte)
*/
unsigned char dataExchangeTest(unsigned char data)
{
  //transmiting data to PLIS on address 0x13
  writeDataOnAddrPlis((unsigned char)0x13, data);

  return getDataByteFromPlis((unsigned char)0x06);
}















