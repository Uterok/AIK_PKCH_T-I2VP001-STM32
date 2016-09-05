#include "ATV_commands.h"
#include "plis_command.h"
#include "emb_flash_read_write.h"


/*---------------------------Errors List----------------------------*/
/*
ATV errors:
  101 - ATV working not from PC
  102 - ATV isn`t ready
  103 - ATV crash
  104 - ATV Blocking

Motion errors:
  111 - wrong engine changed
  112 - slow speed switching error
  113 - motion direction error

Wrong data`s errors:
  121 - predetermined distance is equal to zero


*/





uint8_t moveErr = 0;
uint8_t measureErr = 0;
uint8_t permNextMove = 1;
uint8_t permToMeasure = 0;

float currMeausePoint = 0;

uint8_t measSource = 0;


/*Move to zero
@params: 
detectorNumb - number of the detector which must be reset to zero on the limited switch

@retval: Code of error
  0 - no errors
  101 - ATV working not from PC
  102 - ATV isn`t ready
  103 - ATV crash
  104 - ATV Blocking
  111 - wrong engine changed
  112 - slow speed switching error
  113 - motion direction error
  
*/
unsigned char viezdVNol(unsigned char detectorNumb)
{
  //checking ATV states
  if(getStateBitEVMOrMest() != (unsigned char)1)
  {
    return (unsigned char)101;
  }
  
  if(getStateBitATVReady() != (unsigned char)1)
  {
    return (unsigned char)102;
  }
  
  if(getStateBitATVCrash() == (unsigned char)1)
  {
    return (unsigned char)103;
  }
  
  if(getStateBitATVBlocking() == (unsigned char)1)
  {
    return (unsigned char)104;
  }
  
  //set coordinate detector with parameters
  setCurCoordDet(detectorNumb, (unsigned char)1, (unsigned char)0);
  
  //if detector left of the zero limited switch move to the this switch
  if(getStateBitStopL() != (unsigned char)1)
  {
    switchEngine(getEngNumbFromDetNumb(detectorNumb));
    
    slowSpeedSwitch((unsigned char)0);
    switchEngineSpeed((unsigned char)0x00);
    switchBrakingMode((unsigned char)1);
    motionDirectOfEngine(-1);
    
    while(getStateBitStopL() != (unsigned char)1);  
  }
  
  //stop engines and wait more than 100ms, which need for detector reset
  stopEngines();
  vTaskDelay(110);
  
  //move the switch on slow speed
  switchEngine(getEngNumbFromDetNumb(detectorNumb)); 
  slowSpeedSwitch((unsigned char)1);
  motionDirectOfEngine(1);
  
  //waiting when the limited switch will be triggered off
  while(getStateBitStopL() == (unsigned char)1);
 
  //stop engines and block reset on the limited switch
  stopEngines();
  permResetZCoordDetect(detectorNumb, (unsigned char)0);

  return 0;
}

/*Get direction from calculated distance
@params: 
distance - calculated distance(stop Point - current Point)

@retval: direction to move
  INCREASE - move to increase coordinates(to the left/top)
  DECREASE - move to decrease coordinates(to the right/bottom)
  DIR_ERROR - zero distance 
*/
direction getDirection(float distance)
{
  if(distance > 0)
  {
    return INCREASE;
  }
  else if(distance < 0)
  {
    return DECREASE;
  }
  else
  {
    return DIR_ERROR;
  }
}

/*Get move speed
@params: 
MechanikDatas - structure with mechanick datas
distanceAbs - calculated distance(stop Point - current Point) in absolute coordinates(module from calculated distance)
direction - direction to move
  INCREASE - move to increase coordinates(to the left/top)
  DECREASE - move to decrease coordinates(to the right/bottom)
  DIR_ERROR - zero distance  

@retval: speed to move
  SPEED_100 - 100% speed
  SPEED_75 - 75% speed
  SPEED_50 - 50% speed
  SPEED_25 - 25% speed
  SPEED_SLOW - slow speed
  SPEED_ZERO - error through the zero direction error
*/
speed changeSpeed(MechOnDetect *MechanikDatas,float distanceAbs, direction dir)
{
  if(dir != DIR_ERROR)
  {
    //calculating speed using mechanick values of acceleration and braking
    if(dir == INCREASE)
    {
      //acceleration distance on the speed + braking distance on the speed + way on slow speed + braking on slow speed (must be lower than distance)
      if( (MechanikDatas->tRazgL_100 + MechanikDatas->tTormL_100 + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_slow) <= distanceAbs )
      {
        return SPEED_100;
      }
      else if( (MechanikDatas->tRazgL_75 + MechanikDatas->tTormL_75 + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_slow) <= distanceAbs )
      {
        return SPEED_75;
      }
      else if( (MechanikDatas->tRazgL_50 + MechanikDatas->tTormL_50 + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_slow) <= distanceAbs )
      {
        return SPEED_50;
      }
      else if( (MechanikDatas->tRazgL_25 + MechanikDatas->tTormL_25 + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_slow) <= distanceAbs )
      {
        return SPEED_25;
      }
      else
      {
        return SPEED_SLOW;
      }
    }
    else
    {
      if( (MechanikDatas->tRazgP_100 + MechanikDatas->tTormP_100 + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormP_slow) <= distanceAbs )
      {
        return SPEED_100;
      }
      else if( (MechanikDatas->tRazgP_75 + MechanikDatas->tTormP_75 + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormP_slow) <= distanceAbs )
      {
        return SPEED_75;
      }
      else if( (MechanikDatas->tRazgP_50 + MechanikDatas->tTormP_50 + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormP_slow) <= distanceAbs )
      {
        return SPEED_50;
      }
      else if( (MechanikDatas->tRazgP_25 + MechanikDatas->tTormP_25 + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormP_slow) <= distanceAbs )
      {
        return SPEED_25;
      }
      else
      {
        return SPEED_SLOW;
      }
    }
  }
  else
  {
    return SPEED_ZERO;
  }
}

/*Get detector`s codes from millimeters
@params: 
codesInMM - codes in millimeter(from the mechanical datas)
coordInMM - coordinates in millimeters

@retval: coordinates in detectors codes
*/
long int getCodesFromMM(float codesInMM, float coordInMM)
{
  return (long int)(coordInMM * codesInMM);
}

/*Get millimeters from detector`s codes
@params: 
codesInMM - codes in millimeter(from the mechanical datas)
coordInMM - coordinates in millimeters

@retval: coordinates in millimeters
*/
float getMMfromCodes(float codesInMM, long int coordInCodes)
{
  return (float)coordInCodes / codesInMM;
}

/*Write permissions and points to comparators of the detector
@params: 
MechanikDatas - structure with mechanick datas
stopPoint - stop point
changedSpeed - motion speed
  SPEED_100 - 100% speed
  SPEED_75 - 75% speed
  SPEED_50 - 50% speed
  SPEED_25 - 25% speed
  SPEED_SLOW - slow speed
  SPEED_ZERO - error through the zero direction error
direction - direction to move
  INCREASE - move to increase coordinates(to the left/top)
  DECREASE - move to decrease coordinates(to the right/bottom)
  DIR_ERROR - zero distance
stopPoint - special braking point(braking must begin on this point)
  
@retval: error code
  0 - no errors
  121 - direction error
*/
uint8_t writePermPointsToComp(MechOnDetect *MechanikDatas, float stopPoint, speed changedSpeed, direction dir, float specTormPoint)
{
  if(dir != DIR_ERROR)
  {
    long int stopCoordInCodes, slowSCoordInCodes;
    
    setComparatorsOptions(dir, 0, dir, 0, dir, 0);
    
    if(dir == INCREASE)
    {
      stopCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint - (specTormPoint <= 0 ? MechanikDatas->tTormL_slow : specTormPoint));

      if(changedSpeed == SPEED_100)
      {
        slowSCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint - (MechanikDatas->tTormL_slow + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_100));
      }
      else if(changedSpeed == SPEED_75)
      {
        slowSCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint - (MechanikDatas->tTormL_slow + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_75));
      }
      else if(changedSpeed == SPEED_50)
      {
        slowSCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint - (MechanikDatas->tTormL_slow + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_50));
      }
      else if(changedSpeed == SPEED_25)
      {
        slowSCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint - (MechanikDatas->tTormL_slow + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_25));
      }    
    }
    else
    {
      stopCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint + (specTormPoint <= 0 ? MechanikDatas->tTormP_slow : specTormPoint));
      
       if(changedSpeed == SPEED_100)
      {
        slowSCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint + (MechanikDatas->tTormL_slow + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_100));
      }
      else if(changedSpeed == SPEED_75)
      {
        slowSCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint + (MechanikDatas->tTormL_slow + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_75));
      }
      else if(changedSpeed == SPEED_50)
      {
        slowSCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint + (MechanikDatas->tTormL_slow + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_50));
      }
      else if(changedSpeed == SPEED_25)
      {
        slowSCoordInCodes = getCodesFromMM(MechanikDatas->detCodeInMM, stopPoint + (MechanikDatas->tTormL_slow + MechanikDatas->slowSWayOnTorm + MechanikDatas->tTormL_25));
      }
    }
    
    writeCoordToMotorComp(stopCoordInCodes);
    permOfMotorComp(1);
    
    if(changedSpeed != SPEED_SLOW)
    {
      writeCoordToSlowSpeedComp(slowSCoordInCodes);
      permOfSlowSpeedComp(1);
    }
    
    return 0;
  }
  else
  {
    return 121;
  }
}

/*Check engine switch
@params: 
engineNumber - number of the engine

@retval: changed engine state
  1 - engine switched correct
  255 - engine switched wrong
*/
uint8_t checkEngSwitch(uint8_t engineNumber)
{
  uint8_t engState = 255;
  
  if(engineNumber == 5)
  {
    engState = getStateEngColona();
  }
  else if(engineNumber == 6)
  {
    engState = getStateEngKaretka();
  }
  else if(engineNumber == 2)
  {
    engState = getStateBitEngAzimut();
  }
  
  return engState;
}

/*Check slow speed enable
@params: none

@retval: slow speed state
  0 - slow speed is off
  1 - slow speed is on
*/
uint8_t checkSlowSEnabling()
{
  return getStateBitSlowSpeed();
}

/*Check motion direction
@params: 
direction - selected motion direction
  INCREASE - move to increase coordinates(to the left/top)
  DECREASE - move to decrease coordinates(to the right/bottom)
  DIR_ERROR - zero distance

@retval: changed engine state
  1 - right motion direction
  255 - wrong motion direction
*/
uint8_t checkMotionDir(direction dir)
{
  uint8_t dirState = 255;
  
  if(dir == INCREASE)
  {
    dirState = getStateBitDriveL();
  }
  else if(dir == DECREASE)
  {
    dirState = getStateBitDriveR();
  }
  
  return dirState;
}

/*Move to the point
@params: 
detectorNumb - number of the detector which must be reset to zero on the limited switch
stopPoint - stop point
accuracy - accuracy of the driving up to the point
  <=0 - acurance isn`t required
  >0 - accurance of the driving up to the point
withMeasuring - is motion with measuring
  TRUE - with measuring
  FALSE - without measuring

@retval: Code of error
  0 - no errors
  1 - distance less than accurancy
  101 - ATV working not from PC
  102 - ATV isn`t ready
  103 - ATV crash
  104 - ATV Blocking
  111 - wrong engine changed
  112 - slow speed switching error
  113 - motion direction error
  121 - predetermined distance is equal to zero
*/
unsigned char viezdVTochku(uint8_t detectorNumb, float stopPoint, float accuracy, boolean withMeasuring)
{
  extern MechOnDetect MechanikDatas[QUANT_OF_DETECTORS];
  direction dir;
  speed speedToMove;
  
  //calculate current point in millimeters and find ditance
  float currPoint = (float)getCurrCoordFromDetect() / MechanikDatas[detectorNumb].detCodeInMM;
  float distance = stopPoint - currPoint;
  
  //if distance less than accurancy return error 1
  if((float_abs(distance) <= accuracy) && (accuracy != 0))
  {
    return 1;
  }
  
  //get direction, if direction equal 0 return error 121
  dir = getDirection(distance);
  
  if(dir == DIR_ERROR)
  {
    return 121;
  }
  
  //change speed to moving using mechanich constants distance and direction
  speedToMove = changeSpeed(&MechanikDatas[detectorNumb], distance, dir);
  //write permissions and points to comparators
  uint8_t coordWriteStatus = writePermPointsToComp(&MechanikDatas[detectorNumb], stopPoint, speedToMove, dir, 0);
  
  if(coordWriteStatus != 0)
  {
    return coordWriteStatus;
  }
  
  //run engine on defineted speed to the defineted direction
  uint8_t engineMoveStatus = runEngineWithParams(detectorNumb, speedToMove, dir);
  
  if(engineMoveStatus != 0)
  {
    return engineMoveStatus;
  }
  
  /*----------------!!!!!!!!!!!need mutexes---------------*/
  
  //if previous operations were success denied next motion commands from PC and set move errors equals zero
  moveErr = 0;
  permNextMove = 0;
  
  if(withMeasuring)
  {
    
  }
  
  xTaskCreate(vMoveContr, "transmit", configMINIMAL_STACK_SIZE, (void*)&conn, WEBSERVER_THREAD_PRIO + 3, NULL);
  
  return 0;
  
}


/*Motion control until stop
@params: 
  moveParams - structure with motion parameters(detector number, stop point, accuracy, direction)

@retval: Code of error
  0 - no errors
  114 - no motion in fact
  115 - wrong motion direction in fact
  116 - ATV motion bit has reseted but comparator hasn`t triggered
  117 - comparator has triggered but ATV motion bit hasn`t reseted
  118 - stop or blocking limited switch has triggered
  119 - have drove too big distance after the stop point
  120 - comparator has triggered and ATV motion bit has reseted but motor hasn`t stop
*/
uint8_t moveContrToStop(DataMoveEngine *moveParams)
{
  extern MechOnDetect MechanikDatas[QUANT_OF_DETECTORS];
  
  long int curCoord = 0x00000000;
  long int prevCoord = 0x80000000;
  uint8_t count = 0;
  
  moveErr = 0;
  
  curCoord = getCurrCoordFromDetect();
  
  while((getStateBitStopReq() != 0x01) || (getStateBitDriveR() == 1) || (getStateBitDriveL() == 1))
  {
    //возможно доделать если выезжаем в минуса, но это не актуально, выезд за ноль врядли будет
    
    long int coordDiffer = curCoord - prevCoord;
    
    //if coordinate difference less than part of the codes in mm value motion is absent
    if(lInt_abs(coordDiffer) <= (long int)(MechanikDatas[moveParams->detectorNumb].detCodeInMM / PREC_STOP_DIV_COEF))
    {
      moveErr = 114;
      break;
    }
    
    //checking for the actual motion direction
    if(getDirection(coordDiffer) != moveParams->dir)
    {
      moveErr = 115;
      break;
    }
    
    //if ATV motion bits has reseted but comparator hasn`t triggered
    if((getStateBitStopReq() == 0) && (getStateBitDriveR() == 0) && (getStateBitDriveL() == 0))
    {
      moveErr = 116;
      break;
    }
    
    //if comparator has triggered but ATV motion bits hasn`t reseted  
    if((getStateBitStopReq() == 1) && ((getStateBitDriveR() == 1) || (getStateBitDriveL() == 1)))
    {
      if(count > 1)
      {
        moveErr = 117;
        break;
      }
      else
      {
        count++;
      }
    }
    
    //check triggering stop or blocking limited switches
    if((getStateBitStopR() == 1) || (getStateBitStopL() == 1) || (getStateBitATVBlocking() == 1))
    {
      moveErr = 118;
      break;
    }
    
    //checking the distance after the stop point
    if( (moveParams->stopPoint - getMMfromCodes(MechanikDatas[moveParams->detectorNumb].detCodeInMM, curCoord)) < (float)-MAX_DIST_AFTER_STOP_POINT)
    {
      if(moveParams->dir == INCREASE)
      {
        moveErr = 119;
        break;
      }
    }
    else if((moveParams->stopPoint - getMMfromCodes(MechanikDatas[moveParams->detectorNumb].detCodeInMM, curCoord)) > (float)MAX_DIST_AFTER_STOP_POINT)
    {
      if(moveParams->dir == DECREASE)
      {
        moveErr = 119;
        break;
      }
    }
    //ждать 2 мс для того чтоб знать остановились ли мы изменяется ли координатаы 
    
    vTaskDelay(200);
    
    prevCoord = curCoord;
    curCoord = getCurrCoordFromDetect();
  }
  
  //checking, has the motor really stoped?
  if(moveErr == 0)
  {
    //добавить функцию задержки остановки в зависимости от скорости
    vTaskDelay(500);
  
    for(int i = 0; i < 3; i++)
    {
      if(i == 0)
      {
        prevCoord = getCurrCoordFromDetect();
      }
      else
      {
        curCoord = getCurrCoordFromDetect();
      
        if(lInt_abs(curCoord - prevCoord) > (long int)(MechanikDatas[moveParams->detectorNumb].detCodeInMM / PREC_STOP_DIV_COEF))
        {
          moveErr = 120;
          break;
        }
        
        vTaskDelay(100);
      }
    
    }
  }
  
  //дописать с учетом торможения с больших скоростей(запрос комп остан) и возможного проезда за точку остановки и не срабатывания ничего
  
  //return 0 if no errors
  return moveErr;
}

uint8_t accurateDriveUp()
{
  
}


void vMoveContr(void * pvParameters)
{
  DataMoveEngine moveParams = *(DataMoveEngine *)pvParameters;
  extern MechOnDetect MechanikDatas[QUANT_OF_DETECTORS];

  
  moveErr = moveContrToStop(&moveParams);
  
  
  
  if(moveErr == 0)
  {
    vTaskDelay(1000);
    
    if(moveParams.accuracy != 0)
    {
      float currCoord = getMMfromCodes(MechanikDatas[moveParams.detectorNumb].detCodeInMM, getCurrCoordFromDetect());
      float distance = currCoord - moveParams.stopPoint;
      uint8_t countOfAccurDriveUp = 0;
      
      while((float_abs(distance) > moveParams.accuracy) && (countOfAccurDriveUp < 5) )
      {
        direction dir = getDirection(distance);
        float specTormPoint, allDistance;
        
        //определяем отношение слоу разгона к торм путем сложения и деления торм на сумму, это умножаем на расстояние для подъезда это и будет точка тормож, но не больше пути том с малой скорости, если больше, то точка это путь торможения
        if(dir == INCREASE)
        {
          allDistance = MechanikDatas[moveParams.detectorNumb].tRazgL_slow + MechanikDatas[moveParams.detectorNumb].tTormL_slow;
        }
        else
        {
          allDistance = MechanikDatas[moveParams.detectorNumb].tRazgP_slow + MechanikDatas[moveParams.detectorNumb].tTormP_slow;
        }
        
        if(float_abs(distance) >= allDistance)
        {
          specTormPoint = 0;   
        }
        else
        {
          specTormPoint = float_abs(distance) * ((dir == INCREASE ? MechanikDatas[moveParams.detectorNumb].tTormL_slow : MechanikDatas[moveParams.detectorNumb].tTormP_slow) / allDistance);   
        }
        
        uint8_t coordWriteStatus = writePermPointsToComp(&MechanikDatas[moveParams.detectorNumb], moveParams.stopPoint, SPEED_SLOW, dir, specTormPoint);
              
        if(coordWriteStatus == 1)
        {
          moveErr = 5;
          break;
        }
              
        uint8_t engineMoveStatus = runEngineWithParams(moveParams.detectorNumb, SPEED_SLOW, dir);
              
        if(engineMoveStatus == 1)
        {
          moveErr = 6;
          break;
        }
        else if(engineMoveStatus == 2)
        {
          moveErr = 7;
          break;
        }
        else if(engineMoveStatus == 3)
        {
          moveErr = 8;
          break;
        }
                         
        moveErr = moveContrToStop(&moveParams);
        
        if(moveErr != 0)
        {
          break;
        }
              
        currCoord = getMMfromCodes(MechanikDatas[moveParams.detectorNumb].detCodeInMM, getCurrCoordFromDetect());
        distance = currCoord - moveParams.stopPoint;
        countOfAccurDriveUp++;
        
        vTaskDelay(500);
        
      }
      
        stopEngBecauseOfMist();
    }
  }
  else
  {
    stopEngBecauseOfMist();
  }
  
  permNextMove = 1;
  
  vTaskDelete(NULL);
}


void vMeasuringContr(void * pvParameters)
{
  extern LongWord32 measPoints;
  extern Floats measPointsValues;
  
  if(measPoints.usLength == 0)
  {
    vTaskDelete(NULL);
    return;
  }
  
  measureErr = 0;
  permToMeasure = 0;
  
  DataMeasuring measParams = *(DataMeasuring *)pvParameters;
  
  for(int i = 0; i < measPoints.usLength; i++)
  {
    writeCoordToADCComp(measPoints.plWords[i]);
    dirOfADCComp(measParams.dir);
    permOfADCComp(1);
    
    while(getStateBitADCReq() != 1)//дописать еще ситуацию когда проехали все, но не сработало ничего или начали движение и тут же выскочила ошибка движения
    {
      if((moveErr != 0) && (permNextMove != 0))
      {
        measureErr = 1;
        break;
      }
    }
    
    permOfADCComp(0);
      
    if(measureErr != 0)
    {
      break;
    }
    
    if(measSource == 0)
    {
      
    }
    
    permToMeasure = 1;
    
  }
  
  
  
  
  vTaskDelete(NULL);
}


uint8_t runEngineWithParams(uint8_t detectNumb, speed speedToMove, direction dir)
{
  switchEngine(getEngNumbFromDetNumb(detectNumb));
  
  if(checkEngSwitch(getEngNumbFromDetNumb(detectNumb)) != 1)
  {
    return 101;
  }
  
  if(speedToMove == SPEED_SLOW)
  {
    slowSpeedSwitch(1);
    
    if(checkSlowSEnabling() != 1)
    {
      return 102;
    }
  }
  switchBrakingMode(1);
  
  if((speedToMove != SPEED_SLOW) && (speedToMove != SPEED_ZERO))
  {
    switchEngineSpeed(speedToMove);
  }
  motionDirectOfEngine(dir == INCREASE ? 1 : -1);
  
  vTaskDelay(1);
  
  if(checkMotionDir(dir) != 1)
  {
    return 103;
  } 
  
  return 0;
}

void stopEngBecauseOfMist()
{
  setComparatorsOptions(0, 0, 0, 0, 0, 0);
  stopEngines();
}