#include "stm32f4xx.h"

#define QUANT_OF_DETECTORS         3

#define ADDR_FIRST_TO_WRITE       ((uint32_t)0x080E0000)

#define QUANT_OF_TECH_BYTES        12                              //bytes
#define QUANT_BYTES_IN_COUNTER     25                              //bytes
#define QUANT_IN_EACH_MECH_STR     88                              //bytes

#define QUANT_OF_MECH_STRUCT       0x00004524                      //17700 bytes

#define WORD_IS_ERASED             ADDR_FIRST_TO_WRITE


//Addresses which contains ardessess of beginning definetly current structures
#define ADDR_MECH_KOLONA           ADDR_FIRST_TO_WRITE + QUANT_OF_TECH_BYTES
#define ADDR_MECH_KOLONA_COUNTER   ADDR_MECH_KOLONA
#define ADDR_MECH_KOLONA_DATA      ADDR_MECH_KOLONA_COUNTER + QUANT_BYTES_IN_COUNTER

#define ADDR_MECH_KARETKA           ADDR_MECH_KOLONA + QUANT_OF_MECH_STRUCT
#define ADDR_MECH_KARETKA_COUNTER   ADDR_MECH_KARETKA
#define ADDR_MECH_KARETKA_DATA      ADDR_MECH_KARETKA_COUNTER + QUANT_BYTES_IN_COUNTER

#define ADDR_MECH_AZIMUT            ADDR_MECH_KARETKA  + QUANT_OF_MECH_STRUCT
#define ADDR_MECH_AZIMUT_COUNTER    ADDR_MECH_AZIMUT
#define ADDR_MECH_AZIMUT_DATA       ADDR_MECH_AZIMUT_COUNTER + QUANT_BYTES_IN_COUNTER


typedef enum
{
  FALSE = 0,
  TRUE = 1
}boolean;


typedef struct
{
  boolean isDatasExist;
  float tRazgP_100;
  float tTormP_100;
  float tRazgL_100;
  float tTormL_100;
  float tRazgP_75;
  float tTormP_75;
  float tRazgL_75;
  float tTormL_75;
  float tRazgP_50;
  float tTormP_50;
  float tRazgL_50;
  float tTormL_50;
  float tRazgP_25;
  float tTormP_25;
  float tRazgL_25;
  float tTormL_25;
  float tRazgP_slow;
  float tTormP_slow;
  float tRazgL_slow;
  float tTormL_slow;
  float slowSWayOnTorm;
  float detCodeInMM;
} MechOnDetect;



uint32_t flash_read_Int32(uint32_t address);
uint8_t flash_read_byte(uint32_t address);
float flash_read_float(uint32_t address);

boolean bIsFirstWriting();
void vFLASH_EraseLastSector();
unsigned char usFLASH_ReadDetMechStruct(MechOnDetect *xMechStruct, unsigned char usDetectNumb);
FLASH_Status eFLASH_WriteMechStruct(MechOnDetect xMechStruct, unsigned char usDetectNumb);
FLASH_Status WriteInt32ThroughBytes(uint32_t Address, uint32_t Data);
FLASH_Status WriteFloatThroughBytes(uint32_t Address, float Data);
FLASH_Status IncreaseCounter(uint32_t counterAddress, uint32_t OldCountVal);
FLASH_Status CheckFirstWrite();
uint32_t getCountOfWrites(uint32_t countAddr);
uint32_t getMechDataAddr(unsigned char usDetectNumb);
uint32_t getMechCountAddr(unsigned char usDetectNumb);
void k();
