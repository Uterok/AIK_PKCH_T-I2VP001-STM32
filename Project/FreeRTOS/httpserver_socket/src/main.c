/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; Portions COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */
/**
  ******************************************************************************
  * <h2><center>&copy; Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.</center></h2>
  * @file    main.c
  * @author  CMP Team
  * @version V1.0.0
  * @date    28-December-2012
  * @brief   Main program body      
  *          Modified to support the STM32F4DISCOVERY, STM32F4DIS-BB and
  *          STM32F4DIS-LCD modules. 
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, Embest SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
  * OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
  * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
  * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  ******************************************************************************
  */  
/* Includes ------------------------------------------------------------------*/
#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tcpip.h"
#include "httpserver-socket.h"
#include "plis_command.h"
#include "emb_flash_read_write.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/*--------------- LCD Messages ---------------*/
#define MESSAGE1   "     STM32F4x7      "
#define MESSAGE2   "  STM32F-4 Series   "
#define MESSAGE3   "Basic WebServer Demo"
#define MESSAGE4   "                    "

/*--------------- Tasks Priority -------------*/
#define DHCP_TASK_PRIO   ( tskIDLE_PRIORITY + 2 )      
#define LED_TASK_PRIO    ( tskIDLE_PRIORITY + 1 )

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
xQueueHandle xQueueToTransm;
xSemaphoreHandle xTransmitSemaphore;
xSemaphoreHandle xMutexBytesTransmQueue;
xSemaphoreHandle xMutexAccessToPLIS;
Bytes xMasToTransmQueue[QUEUE_TRANSM_QUANT];

LongWord32 measPoints;

    
/* Private function prototypes -----------------------------------------------*/
void LCD_LED_Init(void);
void initDataFromFlash(void);
void ToggleLed3(void * pvParameters);
void ToggleLed4(void * pvParameters);
void ToggleLed5(void * pvParameters);
void ToggleLed6(void * pvParameters);
void invert(void * pvParameters);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
 int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured to 
       144 MHz, this is done through SystemInit() function which is called from
       startup file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */  

  /*                    Test read/write flash*/
  /*unsigned long int k = 0;
  unsigned long int n = 0;
  MechOnDetect x123, x234;
  
  x123.tRazgP_100 = 1.123;
  x123.tTormP_100 = 2.234;
  x123.tRazgL_100 = 3.345;
  x123.tTormL_100 = 4.456;
  x123.tRazgP_75 = 5.567;
  x123.tTormP_75 = 6.678;
  x123.tRazgL_75 = 7.789;
  x123.tTormL_75 = 8.890;
  x123.tRazgP_50 = 9.901;
  x123.tTormP_50 = 0.001;
  x123.tRazgL_50 = 0.012;
  x123.tTormL_50 = 0.307;
  x123.tRazgP_25 = 0.580;
  x123.tTormP_25 = 0.183;
  x123.tRazgL_25 = 0.408;
  x123.tTormL_25 = 0.0009;
  x123.tRazgP_slow = 0.234;
  x123.tTormP_slow = 1000.0001;
  x123.tRazgL_slow = 19;
  x123.tTormL_slow = 20;
  x123.slowSWayOnTorm = 21;
  x123.detCodeInMM = 22;
  
  FLASH_Status status = FLASH_EraseLastSector();
  if(status != FLASH_COMPLETE)
  {
    int y = 5;
  }
  
  for(int i = 0; i < (QUANT_BYTES_IN_COUNTER * 8) + 1; i++)
  {
    if(i == 200)
    {
      int k = 0;
    }
    
    eFLASH_WriteMechStruct(x123, 0x00);
    //eFLASH_WriteMechStruct(x123, 0x02);
  }
  
  eFLASH_WriteMechStruct(x123, 0x00);
  
  for(int i = 0x080E0000; i < 0x080E000F; i++)
  {
    k = flash_read_Int32(i);
    usFLASH_ReadDetMechStruct(&x234, 0);
    for(int j = 0; i < 32; i++)
    {
      if(((k >> j) & 0x00000001) == 0x00000001)
      {
       n++; 
      }
    }
    
    int f = 0;
    n = 0;
  }*/
  
  /*Initialize LCD and Leds */ 
  LCD_LED_Init();
  
  initOut();
  
  /* configure ethernet (GPIOs, clocks, MAC, DMA) */ 
  ETH_BSP_Config();
    
  /* Initilaize the LwIP stack */
  LwIP_Init();
  
  //Initialize the pins to PLIS
  initPinsToPlis();
  
  //Initialize data from flash memory
  initDataFromFlash();
  
  vSemaphoreCreateBinary(xTransmitSemaphore);
  xQueueToTransm = xQueueCreate(QUEUE_TRANSM_QUANT, sizeof(Bytes *));
  for(int i = 0; i < QUEUE_TRANSM_QUANT; i++)
  {
    vInitStructBytes(&xMasToTransmQueue[i]);
  }
  xMutexBytesTransmQueue = xSemaphoreCreateMutex();
  xMutexAccessToPLIS = xSemaphoreCreateMutex();
  
  /* Initialize webserver demo */
  //http_server_socket_init();
  board_server_socket_init();
  
  

#ifdef USE_DHCP
  /* Start DHCPClient */
  //xTaskCreate(LwIP_DHCP_task, "DHCPClient", configMINIMAL_STACK_SIZE * 2, NULL,DHCP_TASK_PRIO, NULL);
#endif
    
  /* Start toogleLed4 task : Toggle LED4  every 250ms */
  xTaskCreate(ToggleLed3, "LED3", configMINIMAL_STACK_SIZE, NULL, LED_TASK_PRIO, NULL);
  //xHandleLed.leds |= (1 << 3);
  
  xTaskCreate(ToggleLed4, "LED4", configMINIMAL_STACK_SIZE, NULL, LED_TASK_PRIO, NULL);
  //xHandleLed.leds |= (1 << 4);
  
  xTaskCreate(ToggleLed5, "LED5", configMINIMAL_STACK_SIZE, NULL, LED_TASK_PRIO, NULL);
  //xHandleLed.leds |= (1 << 5);
  
  xTaskCreate(ToggleLed6, "LED6", configMINIMAL_STACK_SIZE, NULL, LED_TASK_PRIO, NULL);
  //xHandleLed.leds |= (1 << 6);
  
  //xTaskCreate(invert, "Invert", configMINIMAL_STACK_SIZE, NULL, DHCP_TASK_PRIO, NULL);
  
  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  for ( ;; );
}

void initDataFromFlash()
{
   extern MechOnDetect MechanikDatas[QUANT_OF_DETECTORS];
   
   for(int i = 0; i < QUANT_OF_DETECTORS; i++)
   {
     usFLASH_ReadDetMechStruct(&MechanikDatas[i], i);
   }
}

/**
  * @brief  Toggle Led4 task
  * @param  pvParameters not used
  * @retval None
  */
void invert(void * pvParameters)
{
  for( ;; ){
  invertPort();
  vTaskDelay(2000/portTICK_RATE_MS);
  }
}
    
void ToggleLed3(void * pvParameters)
{
  for ( ;; ) {
    /* Toggle LED3 each 125ms */
    STM_EVAL_LEDToggle(LED3);
    vTaskDelay(125/portTICK_RATE_MS);
  }
}

void ToggleLed4(void * pvParameters)
{
  for ( ;; ) {
    /* Toggle LED4 each 250ms */
    STM_EVAL_LEDToggle(LED4);
    vTaskDelay(250/portTICK_RATE_MS);
  }
}

void ToggleLed5(void * pvParameters)
{
  for ( ;; ) {
    /* Toggle LED5 each 500ms */
    STM_EVAL_LEDToggle(LED5);
    vTaskDelay(500/portTICK_RATE_MS);
  }
}

void ToggleLed6(void * pvParameters)
{
  for ( ;; ) {
    /* Toggle LED6 each 1000ms */
    STM_EVAL_LEDToggle(LED6);
    vTaskDelay(1000);
  }
}

/**
  * @brief  Initializes the LCD and LEDs resources.
  * @param  None
  * @retval None
  */
void LCD_LED_Init(void)
{
#ifdef USE_LCD
  /* Initialize the STM324xG-EVAL's LCD */
  STM32f4_Discovery_LCD_Init();
#endif

  STM_EVAL_LEDInit(LED4); 
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
#ifdef USE_LCD
  /* Clear the LCD */
  LCD_Clear(Black);

  /* Set the LCD Back Color */
  LCD_SetBackColor(Black);

  /* Set the LCD Text Color */
  LCD_SetTextColor(White);

  /* Display message on the LCD*/
  LCD_DisplayStringLine(Line0, (uint8_t*)MESSAGE1);
  LCD_DisplayStringLine(Line1, (uint8_t*)MESSAGE2);
  LCD_DisplayStringLine(Line2, (uint8_t*)MESSAGE3);
  LCD_DisplayStringLine(Line3, (uint8_t*)MESSAGE4); 
#endif
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif


/*********** Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.*****END OF FILE****/
