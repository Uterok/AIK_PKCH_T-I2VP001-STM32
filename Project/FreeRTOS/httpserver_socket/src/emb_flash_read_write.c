#include "emb_flash_read_write.h"


/*
Flash sectors ON STM32F4
0. 0x08000000-0x08003FFF (16 Í¡)
1. 0x08004000-0x08007FFF (16 Í¡)
2. 0x08008000-0x0800BFFF (16 Í¡)
3. 0x0800C000-0x0800FFFF (16 Í¡)
4. 0x08010000-0x0801FFFF (64 Í¡)
5. 0x08020000-0x0803FFFF (128 Í¡)
6. 0x08040000-0x0805FFFF (128 Í¡)
7. 0x08060000-0x0807FFFF (128 Í¡)
8. 0x08080000-0x0809FFFF (128 Í¡)
9. 0x080A0000-0x080BFFFF (128 Í¡)
10. 0x080C0000-0x080DFFFF (128 Í¡)
11. 0x080E0000-0x080FFFFF (128 Í¡)
*/

/* Global variables ---------------------------------------------------------*/
MechOnDetect MechanikDatas[QUANT_OF_DETECTORS];

/*Function for test
void k()
{
  MechOnDetect k[QUANT_OF_DETECTORS];
  
  for(int i = 0; i < QUANT_OF_DETECTORS + 1; i++)
  {
    k[i] = MechanikDatas[i];
  }
}*/

/*Read byte from memory(flash)
@params:
address - adress for reading

@retval: byte, which has read from memory 
*/
uint8_t flash_read_byte(uint32_t address)
{
  return (*(__IO uint8_t*) address);
}
  
/*Read word from memory(flash)
@params:
address - adress for reading

@retval: byte, which has read from memory 
*/
uint32_t flash_read_Int32(uint32_t address)
{
  return (*(__IO uint32_t*) address);
}

/*Read float from memory(flash)
@params:
address - adress for reading

@retval: word, which has read from memory 
*/
float flash_read_float(uint32_t address)
{
  return (*(__IO float*) address);
}



/*Checking for first writing in flash memory page
@params: none
@retval: first writing flag
  TRUE - first writing to flash memory page
  FALSE - flash memory already have wrote
*/
boolean bIsFirstWriting()
{
  if(flash_read_Int32(WORD_IS_ERASED) != (uint32_t)0x00000000)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/*Erasing Sector 11 in STM32F4
@params: none
@retval: status of the flash memory operation ( enum FLASH_Status in stm32f4xx_flash.h)
*/
FLASH_Status FLASH_EraseLastSector()
{
  FLASH_Status status = FLASH_COMPLETE;
  
  FLASH_Unlock();
  
  status = FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
  
  FLASH_Lock();
  
  return status;
}

/*Rewrite all datas in data count overloading
@params: none
@retval: status of the flash memory operation ( enum FLASH_Status in stm32f4xx_flash.h)
*/
FLASH_Status RewriteOnCountOverload()
{
  FLASH_Status status = FLASH_COMPLETE;
  MechOnDetect MechDatas[QUANT_OF_DETECTORS];
  unsigned long int existDatas[QUANT_OF_DETECTORS];
  
  for(int i = 0; i < QUANT_OF_DETECTORS; i++)
  {
    if(usFLASH_ReadDetMechStruct(&MechDatas[i], i) == 1)
    {
      existDatas[i] = 0;
    }
    else if(usFLASH_ReadDetMechStruct(&MechDatas[i], i) == 0)
    {
      existDatas[i] = 1;
    }
  }
  
  FLASH_EraseLastSector();
  
  for(int i = 0; i < QUANT_OF_DETECTORS; i++)
  {
    if(existDatas[i] == 1)
    {
      status = eFLASH_WriteMechStruct(MechDatas[i], i);
      if(status != FLASH_COMPLETE)
      {
        return status;
      }
    }
  }
  
  return status;
}

/*Get mechanick datas of the detector from the flash memory
@params:
xMechStruct - structure of machanick datas(reference value)

@retval: reading status
  0 - datas have read succesfully
  1 - mechanick datas of this detector are absent
*/
unsigned char usFLASH_ReadDetMechStruct(MechOnDetect *xMechStruct, unsigned char usDetectNumb)
{
  unsigned long int ulAddrStructBegin, count, countOfWrites;
  
  countOfWrites = getCountOfWrites(getMechCountAddr(usDetectNumb));
  if(countOfWrites == 0)
  {
    xMechStruct->isDatasExist = FALSE;
    return 1;
  }
  
  ulAddrStructBegin = getMechDataAddr(usDetectNumb) + ((countOfWrites - 1) * QUANT_IN_EACH_MECH_STR);
  
  count = 0;
  
  xMechStruct->tRazgP_100 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormP_100 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tRazgL_100 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormL_100 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tRazgP_75 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormP_75 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tRazgL_75 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormL_75 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tRazgP_50 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormP_50 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tRazgL_50 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormL_50 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tRazgP_25 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormP_25 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tRazgL_25 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormL_25 = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tRazgP_slow = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormP_slow = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tRazgL_slow = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->tTormL_slow = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->slowSWayOnTorm = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  xMechStruct->detCodeInMM = flash_read_float((uint32_t)(ulAddrStructBegin + (count * 4)));
  count++;
  
  xMechStruct->isDatasExist = TRUE;
  
  return 0;
}

/*Write all datas of detector`s mechanick in flash memory 
@params:
xMechStruct - structure with datas
usDetectNumb - detectors number
  0 - kolona
  1 - karetka
  2 - azimut
@retval: status of the flash memory operation ( enum FLASH_Status in stm32f4xx_flash.h)
*/
FLASH_Status eFLASH_WriteMechStruct(MechOnDetect xMechStruct, unsigned char usDetectNumb)
{
  FLASH_Status status = FLASH_COMPLETE;
  unsigned long int count = 0;
  
  uint32_t countOfWrites = getCountOfWrites(getMechCountAddr(usDetectNumb));
  
  if(countOfWrites >= QUANT_BYTES_IN_COUNTER * 8)
  {
    status = RewriteOnCountOverload();
    if(status != FLASH_COMPLETE)
    {
      return status;
    }
  }
  
  status = CheckFirstWrite();
  if(status != FLASH_COMPLETE)
  {
    return status;
  }

  uint32_t countBeginAddr = getMechCountAddr(usDetectNumb);
  
  uint32_t addrBeginWrite = getMechDataAddr(usDetectNumb) + (countOfWrites * QUANT_IN_EACH_MECH_STR);
  
  count = 0;
  
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgP_100);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormP_100);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgL_100);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormL_100);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgP_75);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormP_75);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgL_75);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormL_75);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgP_50);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormP_50);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgL_50);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormL_50);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgP_25);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormP_25);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgL_25);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormL_25);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgP_slow);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormP_slow);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tRazgL_slow);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.tTormL_slow);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.slowSWayOnTorm);
  count++;
  status = WriteFloatThroughBytes(addrBeginWrite + (count * 4), xMechStruct.detCodeInMM);
  
  IncreaseCounter(countBeginAddr, countOfWrites);
  
  return status;
}

/*Get adress of the mechanik datas block
@params:
usDetectNumb - detectors number
  0 - kolona
  1 - karetka
  2 - azimut
@retval: adress of the mechanik datas block
  ADDR_MECH_KOLONA_DATA - address of the kolona data`s block
  ADDR_MECH_KARETKA_DATA - address of the karetka data`s block
  ADDR_MECH_AZIMUT_DATA - address of the azimut data`s block
*/
uint32_t getMechDataAddr(unsigned char usDetectNumb)
{
  if(usDetectNumb == 0x00)
  {
    return ADDR_MECH_KOLONA_DATA;
  }
  else if(usDetectNumb == 0x01)
  {
    return ADDR_MECH_KARETKA_DATA;
  }
  else if(usDetectNumb == 0x02)
  {
    return ADDR_MECH_AZIMUT_DATA;
  }
}

/*Get adress of the mechanik datas counter
@params:
usDetectNumb - detectors number
  0 - kolona
  1 - karetka
  2 - azimut
@retval: adress of the mechanik datas block
  ADDR_MECH_KOLONA_COUNTER - address of the kolona data`s counter
  ADDR_MECH_KARETKA_COUNTER - address of the karetka data`s counter
  ADDR_MECH_AZIMUT_COUNTER - address of the azimut data`s counter
*/
uint32_t getMechCountAddr(unsigned char usDetectNumb)
{
  if(usDetectNumb == 0x00)
  {
    return ADDR_MECH_KOLONA_COUNTER;
  }
  else if(usDetectNumb == 0x01)
  {
    return ADDR_MECH_KARETKA_COUNTER;
  }
  else if(usDetectNumb == 0x02)
  {
    return ADDR_MECH_AZIMUT_COUNTER;
  }
}

/*Get quantaty of writes from counter
@params:
countAddr - address of the counter

@retval: counter value(0 ... (8 * QUANT_BYTES_IN_COUNTER))
*/
uint32_t getCountOfWrites(uint32_t countAddr)
{
  unsigned long int countOfWrites = 0;
  unsigned char byteFromFlash = 0;
  
  for(int i = 0; i < QUANT_BYTES_IN_COUNTER; i++)
  {
    byteFromFlash = (unsigned char)flash_read_byte(countAddr + i);
    if(byteFromFlash == 0x00)
    {
      countOfWrites += 8;
    }
    else
    {
      for(int j = 0; j < 8; j++)
      {
        if(((byteFromFlash >> j) & 0x01) == 0x00)
        {
          countOfWrites++;
        }
        else
        {
          break;
        }
      }
      
      break;
    } 
  }
  
  return countOfWrites;
}

/*Checking for first writing in this flash memory page
@params: none

@retval: status of the flash memory operation ( enum FLASH_Status in stm32f4xx_flash.h)
*/
FLASH_Status CheckFirstWrite()
{
  FLASH_Status status = FLASH_COMPLETE;
  
  if(bIsFirstWriting() == TRUE)
  {
    status = FLASH_EraseLastSector();
    if(status != FLASH_COMPLETE)
    {
      return status;
    }
    
    status = WriteInt32ThroughBytes(WORD_IS_ERASED, 0x00000000);
    if(status != FLASH_COMPLETE)
    {
      return status;
    }
  }
  
  return status;
}

/*Increase the counter after sucsessfully writing datas to flash memory
@params:
counterAddress - counter address
OldCountVal - old counter value(0 ... (8 * QUANT_BYTES_IN_COUNTER) - 1)

@retval: status of the flash memory operation ( enum FLASH_Status in stm32f4xx_flash.h)
*/
FLASH_Status IncreaseCounter(uint32_t counterAddress, uint32_t OldCountVal)
{
  FLASH_Status status = FLASH_COMPLETE;
  
  FLASH_Unlock();
  
  status = FLASH_ProgramByte(counterAddress + (OldCountVal / 8), (unsigned int)0xff << ((OldCountVal % 8) + 1));
  
  FLASH_Lock();
  
  return status;
}

/*Write Int32 byte by byte
@params:
Address - address to byte writing
Data - datas to writing

@retval: status of the flash memory operation ( enum FLASH_Status in stm32f4xx_flash.h)
*/
FLASH_Status WriteInt32ThroughBytes(uint32_t Address, uint32_t Data)
{
  FLASH_Status status;

  FLASH_Unlock();

  for(int i = 0; i < 4; i++)
  {  
    status = FLASH_ProgramByte(Address + i, (uint8_t)((Data >> (i * 8)) & 0x000000ff));
    
    if(status != FLASH_COMPLETE)
    {
      break;
    }
  }
  
  FLASH_Lock();
  
  return status;
}


/*Write float byte by byte
@params:
Address - address to byte writing
Data - datas to writing

@retval: status of the flash memory operation ( enum FLASH_Status in stm32f4xx_flash.h)
*/
FLASH_Status WriteFloatThroughBytes(uint32_t Address, float Data)
{
  FLASH_Status status;
  
  FLASH_Unlock();

  for(int i = 0; i < 4; i++)
  {  
    status = FLASH_ProgramByte(Address + i, (uint8_t)(( (*(uint32_t *)&Data) >> (i * 8)) & 0x000000ff));
    
    if(status != FLASH_COMPLETE)
    {
      break;
    }
  }
  
  FLASH_Lock();
  
  return status;
}

