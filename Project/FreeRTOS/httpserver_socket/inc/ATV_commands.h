#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "plis_command.h"
#include "httpserver-socket.h"


/*---------------Constants definitions----------------*/
#define MAX_DIST_AFTER_STOP_POINT             5
#define MAX_STOP_ACCURACY                     (float)0.05
#define PREC_STOP_DIV_COEF                    10                //division coefficient to find 100% engine stop


typedef enum
{
  SPEED_100 = 0,
  SPEED_75 = 1,
  SPEED_50,
  SPEED_25,
  SPEED_SLOW,
  SPEED_ZERO
}speed;

typedef enum
{
  INCREASE = 0,
  DECREASE = 1,
  DIR_ERROR
}direction;

typedef struct
{
  uint8_t detectorNumb;
  float stopPoint;
  float accuracy;
  direction dir;
  speed speedToMove;
}DataMoveEngine;

typedef struct
{
  direction dir;
}DataMeasuring;





uint8_t runEngineWithParams(uint8_t detectNumb, speed speedToMove, direction dir);
void stopEngBecauseOfMist();



void vMoveContr(void * pvParameters);


