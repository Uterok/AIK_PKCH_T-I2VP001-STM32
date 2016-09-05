/**
  ******************************************************************************
  * @file    httpserver-socket.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011  
  * @brief   Basic http server implementation using LwIP socket API   
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
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */
  
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "fs.h"
#include "fsdata.h"
#include "string.h"
#include "httpserver-socket.h"
#include "plis_command.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define WEBSERVER_THREAD_PRIO    ( tskIDLE_PRIORITY + 3 )

#define CODE_OF_ERROR 0
#define CODE_OF_OPERATION 1
#define PACK_LENGHT_FIRST_BYTE 2
#define PACK_LENGHT_SECOND_BYTE 3

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u32_t nPageHits = 0;
portCHAR PAGE_BODY[512];

unsigned char recvErrReg = 0x00000000;

unsigned char ucCoordInCycle = 0;
unsigned char ucStateInCycle = 0;

unsigned char pcInQueue[256];

extern xSemaphoreHandle xTransmitSemaphore;
extern xSemaphoreHandle xMutexBytesTransmQueue;
extern xSemaphoreHandle xMutexAccessToPLIS;
extern xQueueHandle xQueueToTransm;

extern Bytes xMasToTransmQueue[QUEUE_TRANSM_QUANT];


/* Format of dynamic web page: the page header */
static const unsigned char PAGE_START[] = {
0x3c,0x21,0x44,0x4f,0x43,0x54,0x59,0x50,0x45,0x20,0x68,0x74,0x6d,0x6c,0x20,0x50,
0x55,0x42,0x4c,0x49,0x43,0x20,0x22,0x2d,0x2f,0x2f,0x57,0x33,0x43,0x2f,0x2f,0x44,
0x54,0x44,0x20,0x48,0x54,0x4d,0x4c,0x20,0x34,0x2e,0x30,0x31,0x2f,0x2f,0x45,0x4e,
0x22,0x20,0x22,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x77,0x77,0x77,0x2e,0x77,0x33,
0x2e,0x6f,0x72,0x67,0x2f,0x54,0x52,0x2f,0x68,0x74,0x6d,0x6c,0x34,0x2f,0x73,0x74,
0x72,0x69,0x63,0x74,0x2e,0x64,0x74,0x64,0x22,0x3e,0x0d,0x0a,0x3c,0x68,0x74,0x6d,
0x6c,0x3e,0x0d,0x0a,0x3c,0x68,0x65,0x61,0x64,0x3e,0x0d,0x0a,0x20,0x20,0x3c,0x74,
0x69,0x74,0x6c,0x65,0x3e,0x53,0x54,0x4d,0x33,0x32,0x46,0x34,0x78,0x37,0x54,0x41,
0x53,0x4b,0x53,0x3c,0x2f,0x74,0x69,0x74,0x6c,0x65,0x3e,0x0d,0x0a,0x20,0x20,0x3c,
0x6d,0x65,0x74,0x61,0x20,0x68,0x74,0x74,0x70,0x2d,0x65,0x71,0x75,0x69,0x76,0x3d,
0x22,0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x54,0x79,0x70,0x65,0x22,0x0d,0x0a,
0x20,0x63,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x3d,0x22,0x74,0x65,0x78,0x74,0x2f,0x68,
0x74,0x6d,0x6c,0x3b,0x20,0x63,0x68,0x61,0x72,0x73,0x65,0x74,0x3d,0x77,0x69,0x6e,
0x64,0x6f,0x77,0x73,0x2d,0x31,0x32,0x35,0x32,0x22,0x3e,0x0d,0x0a,0x20,0x20,0x3c,
0x6d,0x65,0x74,0x61,0x20,0x68,0x74,0x74,0x70,0x2d,0x65,0x71,0x75,0x69,0x76,0x3d,
0x22,0x72,0x65,0x66,0x72,0x65,0x73,0x68,0x22,0x20,0x63,0x6f,0x6e,0x74,0x65,0x6e,
0x74,0x3d,0x22,0x31,0x22,0x3e,0x0d,0x0a,0x20,0x20,0x3c,0x6d,0x65,0x74,0x61,0x20,
0x63,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x3d,0x22,0x4d,0x53,0x48,0x54,0x4d,0x4c,0x20,
0x36,0x2e,0x30,0x30,0x2e,0x32,0x38,0x30,0x30,0x2e,0x31,0x35,0x36,0x31,0x22,0x20,
0x6e,0x61,0x6d,0x65,0x3d,0x22,0x47,0x45,0x4e,0x45,0x52,0x41,0x54,0x4f,0x52,0x22,
0x3e,0x0d,0x0a,0x20,0x20,0x3c,0x73,0x74,0x79,0x6c,0x65,0x20,0x3d,0x22,0x66,0x6f,
0x6e,0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x6e,0x6f,0x72,0x6d,0x61,
0x6c,0x3b,0x20,0x66,0x6f,0x6e,0x74,0x2d,0x66,0x61,0x6d,0x69,0x6c,0x79,0x3a,0x20,
0x56,0x65,0x72,0x64,0x61,0x6e,0x61,0x3b,0x22,0x3e,0x3c,0x2f,0x73,0x74,0x79,0x6c,
0x65,0x3e,0x0d,0x0a,0x3c,0x2f,0x68,0x65,0x61,0x64,0x3e,0x0d,0x0a,0x3c,0x62,0x6f,
0x64,0x79,0x3e,0x0d,0x0a,0x3c,0x68,0x34,0x3e,0x3c,0x73,0x6d,0x61,0x6c,0x6c,0x20,
0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x66,0x61,0x6d,0x69,
0x6c,0x79,0x3a,0x20,0x56,0x65,0x72,0x64,0x61,0x6e,0x61,0x3b,0x22,0x3e,0x3c,0x73,
0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x62,0x69,0x67,0x3e,0x3c,0x62,0x69,0x67,0x3e,0x3c,
0x62,0x69,0x67,0x0d,0x0a,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,
0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x22,
0x3e,0x3c,0x62,0x69,0x67,0x3e,0x3c,0x73,0x74,0x72,0x6f,0x6e,0x67,0x3e,0x3c,0x65,
0x6d,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x0d,0x0a,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,
0x22,0x66,0x6f,0x6e,0x74,0x2d,0x73,0x74,0x79,0x6c,0x65,0x3a,0x20,0x69,0x74,0x61,
0x6c,0x69,0x63,0x3b,0x22,0x3e,0x53,0x54,0x4d,0x33,0x32,0x46,0x34,0x78,0x37,0x20,
0x4c,0x69,0x73,0x74,0x20,0x6f,0x66,0x20,0x74,0x61,0x73,0x6b,0x73,0x20,0x61,0x6e,
0x64,0x0d,0x0a,0x74,0x68,0x65,0x69,0x72,0x20,0x73,0x74,0x61,0x74,0x75,0x73,0x3c,
0x2f,0x73,0x70,0x61,0x6e,0x3e,0x3c,0x2f,0x65,0x6d,0x3e,0x3c,0x2f,0x73,0x74,0x72,
0x6f,0x6e,0x67,0x3e,0x3c,0x2f,0x62,0x69,0x67,0x3e,0x3c,0x2f,0x62,0x69,0x67,0x3e,
0x3c,0x2f,0x62,0x69,0x67,0x3e,0x3c,0x2f,0x62,0x69,0x67,0x3e,0x3c,0x2f,0x73,0x6d,
0x61,0x6c,0x6c,0x3e,0x3c,0x2f,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x2f,0x68,0x34,
0x3e,0x0d,0x0a,0x3c,0x68,0x72,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x77,0x69,
0x64,0x74,0x68,0x3a,0x20,0x31,0x30,0x30,0x25,0x3b,0x20,0x68,0x65,0x69,0x67,0x68,
0x74,0x3a,0x20,0x32,0x70,0x78,0x3b,0x22,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x0d,0x0a,
0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x77,0x65,0x69,
0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x22,0x3e,0x0d,0x0a,0x3c,0x2f,
0x73,0x70,0x61,0x6e,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x20,0x73,0x74,0x79,0x6c,0x65,
0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,
0x6f,0x6c,0x64,0x3b,0x22,0x3e,0x0d,0x0a,0x3c,0x74,0x61,0x62,0x6c,0x65,0x20,0x73,
0x74,0x79,0x6c,0x65,0x3d,0x22,0x77,0x69,0x64,0x74,0x68,0x3a,0x20,0x39,0x36,0x31,
0x70,0x78,0x3b,0x20,0x68,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x33,0x30,0x70,0x78,
0x3b,0x22,0x20,0x62,0x6f,0x72,0x64,0x65,0x72,0x3d,0x22,0x31,0x22,0x0d,0x0a,0x20,
0x63,0x65,0x6c,0x6c,0x70,0x61,0x64,0x64,0x69,0x6e,0x67,0x3d,0x22,0x32,0x22,0x20,
0x63,0x65,0x6c,0x6c,0x73,0x70,0x61,0x63,0x69,0x6e,0x67,0x3d,0x22,0x32,0x22,0x3e,
0x0d,0x0a,0x20,0x20,0x3c,0x74,0x62,0x6f,0x64,0x79,0x3e,0x0d,0x0a,0x20,0x20,0x20,
0x20,0x3c,0x74,0x72,0x3e,0x0d,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x3c,0x74,0x64,
0x0d,0x0a,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x66,
0x61,0x6d,0x69,0x6c,0x79,0x3a,0x20,0x56,0x65,0x72,0x64,0x61,0x6e,0x61,0x3b,0x20,
0x66,0x6f,0x6e,0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,
0x64,0x3b,0x20,0x66,0x6f,0x6e,0x74,0x2d,0x73,0x74,0x79,0x6c,0x65,0x3a,0x20,0x69,
0x74,0x61,0x6c,0x69,0x63,0x3b,0x20,0x62,0x61,0x63,0x6b,0x67,0x72,0x6f,0x75,0x6e,
0x64,0x2d,0x63,0x6f,0x6c,0x6f,0x72,0x3a,0x20,0x72,0x67,0x62,0x28,0x35,0x31,0x2c,
0x20,0x35,0x31,0x2c,0x20,0x32,0x35,0x35,0x29,0x3b,0x20,0x74,0x65,0x78,0x74,0x2d,
0x61,0x6c,0x69,0x67,0x6e,0x3a,0x20,0x63,0x65,0x6e,0x74,0x65,0x72,0x3b,0x22,0x3e,
0x3c,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x61,0x0d,0x0a,0x20,0x68,0x72,0x65,0x66,
0x3d,0x22,0x2f,0x53,0x54,0x4d,0x33,0x32,0x46,0x34,0x78,0x37,0x2e,0x68,0x74,0x6d,
0x6c,0x22,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,
0x63,0x6f,0x6c,0x6f,0x72,0x3a,0x20,0x77,0x68,0x69,0x74,0x65,0x3b,0x22,0x3e,0x48,
0x6f,0x6d,0x65,0x0d,0x0a,0x70,0x61,0x67,0x65,0x3c,0x2f,0x73,0x70,0x61,0x6e,0x3e,
0x3c,0x2f,0x61,0x3e,0x3c,0x2f,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x2f,0x74,0x64,
0x3e,0x0d,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x3c,0x74,0x64,0x0d,0x0a,0x20,0x73,
0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x66,0x61,0x6d,0x69,0x6c,
0x79,0x3a,0x20,0x56,0x65,0x72,0x64,0x61,0x6e,0x61,0x3b,0x20,0x66,0x6f,0x6e,0x74,
0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x20,0x66,
0x6f,0x6e,0x74,0x2d,0x73,0x74,0x79,0x6c,0x65,0x3a,0x20,0x69,0x74,0x61,0x6c,0x69,
0x63,0x3b,0x20,0x62,0x61,0x63,0x6b,0x67,0x72,0x6f,0x75,0x6e,0x64,0x2d,0x63,0x6f,
0x6c,0x6f,0x72,0x3a,0x20,0x72,0x67,0x62,0x28,0x35,0x31,0x2c,0x20,0x35,0x31,0x2c,
0x20,0x32,0x35,0x35,0x29,0x3b,0x20,0x74,0x65,0x78,0x74,0x2d,0x61,0x6c,0x69,0x67,
0x6e,0x3a,0x20,0x63,0x65,0x6e,0x74,0x65,0x72,0x3b,0x22,0x3e,0x3c,0x61,0x0d,0x0a,
0x20,0x68,0x72,0x65,0x66,0x3d,0x22,0x53,0x54,0x4d,0x33,0x32,0x46,0x34,0x78,0x37,
0x41,0x44,0x43,0x2e,0x68,0x74,0x6d,0x6c,0x22,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x20,
0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x77,0x65,0x69,0x67,
0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x22,0x3e,0x3c,0x2f,0x73,0x70,0x61,
0x6e,0x3e,0x3c,0x2f,0x61,0x3e,0x3c,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x61,0x0d,
0x0a,0x20,0x68,0x72,0x65,0x66,0x3d,0x22,0x2f,0x53,0x54,0x4d,0x33,0x32,0x46,0x34,
0x78,0x37,0x54,0x41,0x53,0x4b,0x53,0x2e,0x68,0x74,0x6d,0x6c,0x22,0x3e,0x3c,0x73,
0x70,0x61,0x6e,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x63,0x6f,0x6c,0x6f,0x72,
0x3a,0x20,0x77,0x68,0x69,0x74,0x65,0x3b,0x22,0x3e,0x4c,0x69,0x73,0x74,0x0d,0x0a,
0x6f,0x66,0x20,0x74,0x61,0x73,0x6b,0x73,0x3c,0x2f,0x73,0x70,0x61,0x6e,0x3e,0x3c,
0x2f,0x61,0x3e,0x3c,0x2f,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x2f,0x74,0x64,0x3e,
0x0d,0x0a,0x20,0x20,0x20,0x20,0x3c,0x2f,0x74,0x72,0x3e,0x0d,0x0a,0x20,0x20,0x3c,
0x2f,0x74,0x62,0x6f,0x64,0x79,0x3e,0x0d,0x0a,0x3c,0x2f,0x74,0x61,0x62,0x6c,0x65,
0x3e,0x0d,0x0a,0x3c,0x62,0x72,0x3e,0x0d,0x0a,0x3c,0x2f,0x73,0x70,0x61,0x6e,0x3e,
0x3c,0x73,0x70,0x61,0x6e,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,
0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x22,
0x3e,0x3c,0x2f,0x73,0x70,0x61,0x6e,0x3e,0x3c,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,
0x73,0x70,0x61,0x6e,0x0d,0x0a,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,
0x6e,0x74,0x2d,0x66,0x61,0x6d,0x69,0x6c,0x79,0x3a,0x20,0x56,0x65,0x72,0x64,0x61,
0x6e,0x61,0x3b,0x22,0x3e,0x4e,0x75,0x6d,0x62,0x65,0x72,0x20,0x6f,0x66,0x20,0x70,
0x61,0x67,0x65,0x20,0x68,0x69,0x74,0x73,0x3a,0x0d,0x0a,0x00};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief serve tcp connection  
  * @param conn: connection socket 
  * @retval None
  */
void http_server_serve(int conn) 
{
  int buflen= 1500;
  int ret;
  struct fs_file * file;
  unsigned char recv_buffer[1500];
  
  char* d = "I'm new to C# and I wondered how to convert an Array of byte to array chars? Provided that I have this array. byte[] a = new byte[50];";
				
  /* Read in the request */
  ret = read(conn, (void**)&recv_buffer, buflen); 
  if(ret < 0) return;

  /* Check if request to get ST.gif */
  if (strncmp((char *)recv_buffer,"GET /STM32F4x7_files/ST.gif",27)==0)
  {
    file = fs_open("/STM32F4x7_files/ST.gif"); 
    write(conn, (const unsigned char*)(file->data), (size_t)file->len);
    if(file) fs_close(file);
  }
  /* Check if request to get stm32.jpeg */
  else if (strncmp((char *)recv_buffer,"GET /STM32F4x7_files/stm32.jpg",30)==0)
  {
    file = fs_open("/STM32F4x7_files/stm32.jpg"); 
    write(conn, (const unsigned char*)(file->data), (size_t)file->len);
    if(file) fs_close(file);
  }
  /* Check if request to get ST logo.jpeg */
  else if (strncmp((char *)recv_buffer,"GET /STM32F4x7_files/logo.jpg", 29) == 0)
  {
    file = fs_open("/STM32F4x7_files/logo.jpg"); 
    write(conn, (const unsigned char*)(file->data), (size_t)file->len);
    if(file) fs_close(file);
  }
  else if(strncmp((char *)recv_buffer, "GET /STM32F4x7TASKS.html", 24) == 0)
  {
    /* Load dynamic page */
    DynWebPage(conn);
  }
  else if((strncmp((char *)recv_buffer, "GET /STM32F4x7.html", 19) == 0)||(strncmp((char *)recv_buffer, "GET / ", 6) == 0))
  {
    /* Load STM32F4x7 page */
    file = fs_open("/STM32F4x7.html"); 
    write(conn, (const unsigned char*)(file->data), (size_t)file->len);
    if(file) fs_close(file);
  }
  else if(strncmp((char *)recv_buffer, "345", 3) == 0)
  {
    /* Load STM32F4x7 page */
    file = fs_open("/STM32F4x7_files/logo.jpg"); 
    write(conn, (const unsigned char*)d, (size_t)strlen(d));
    if(file) fs_close(file);
  }
  else
  {
    /* Load 404 page */
    file = fs_open("/404.html"); 
    //write(conn, (const unsigned char*)(file->data), (size_t)file->len);
    
    write(conn, (char *)"this is message", 15);
    if(file) fs_close(file);
  }
  /* Close connection socket */
  close(conn);
}

/**
  * @brief  http server thread 
  * @param arg: pointer on argument(not used here) 
  * @retval None
  */
static void http_server_socket_thread(void *arg)
{
  int sock, newconn, size;
  struct sockaddr_in address, remotehost;

 /* create a TCP socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
  {
    printf("can not create socket");
    return;
  }
  
  /* bind to port 80 at any interface */
  address.sin_family = AF_INET;
  address.sin_port = htons(80);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
  {
    printf("can not bind socket");
    return;
  }
  
  /* listen for incoming connections (TCP listen backlog = 5) */
  listen(sock, 5);
  
  size = sizeof(remotehost);
  
  while (1) 
  {
    newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    http_server_serve(newconn);
  }
}

/**
  * @brief  Initialize the HTTP server (start its thread) 
  * @param  none
  * @retval None
  */
void http_server_socket_init()
{
  sys_thread_new("HTTP", http_server_socket_thread, NULL, DEFAULT_THREAD_STACKSIZE * 2, WEBSERVER_THREAD_PRIO);
}

/**
  * @brief  Create and send a dynamic Web Page. This page contains the list of 
  *         running tasks and the number of page hits. 
  * @param  conn connection socket
  * @retval None
  */
void DynWebPage(int conn)
{
  portCHAR pagehits[10];

  memset(PAGE_BODY, 0,512);

  /* Update the hit count */
  nPageHits++;
  sprintf( pagehits, "%d", nPageHits );
  strcat(PAGE_BODY, pagehits);
  strcat((char *) PAGE_BODY, "<pre><br>Name          State  Priority  Stack   Num" );
  strcat((char *) PAGE_BODY, "<br>---------------------------------------------<br>"); 
    
  /* The list of tasks and their status */
  vTaskList((signed char *)(PAGE_BODY + strlen(PAGE_BODY)));
  strcat((char *) PAGE_BODY, "<br><br>---------------------------------------------"); 
  strcat((char *) PAGE_BODY, "<br>B : Blocked, R : Ready, D : Deleted, S : Suspended<br>");
  
  /* Send the dynamically generated page */
  write(conn, PAGE_START, strlen((char*)PAGE_START));
  write(conn, PAGE_BODY, strlen(PAGE_BODY));
}





//----------------------TASK OF CONTROL BOARD---------------------------------------------------------------------------------//

//----Creating/destroing struct of data---------------

//Initializator of structure words32
void vInitStructDataWordsX32(LongWord32 *pxDataMas)
{
  pxDataMas->usLength = 0;
  pxDataMas->plWords = NULL;
}

//Destructor of structure words32
void vDestroyStructDataWors32(LongWord32 *pxDataMas)
{
  if(pxDataMas->usLength != 0)
  {
    vPortFree(pxDataMas->plWords);
    pxDataMas->plWords = NULL;
    pxDataMas->usLength = 0;
  }
}

//Constructor of structure words32
void vCreateStructDataWordsX32(LongWord32 *pxDataMas, unsigned int sLength)
{
  if(pxDataMas->usLength > 0)
  {
    vDestroyStructDataWors32(pxDataMas);
  }

  pxDataMas->usLength = sLength;
  pxDataMas->plWords = (long int*)pvPortMalloc((pxDataMas->usLength)*sizeof(long int));
  for(int i = 0; i < pxDataMas->usLength; i++)
  {
    pxDataMas->plWords[i] = 0;
  }
}

//Initializator of structure bytes
void vInitStructBytes(Bytes *pxStructByte)
{
  pxStructByte->usLength = 0;
  pxStructByte->pcBytes = NULL;
}

//Destructor of structure bytes
void vDestroyStructBytes(Bytes *pxStructByte)
{
  if(pxStructByte->usLength != 0)
  {
    vPortFree(pxStructByte->pcBytes);
    pxStructByte->pcBytes = NULL;
    pxStructByte->usLength = 0;
  }
}

//Constructor of structure bytes
void vCreateStructBytes(Bytes *pxStructByte, unsigned int sLength)
{
  if(pxStructByte->usLength > 0)
  {
    vDestroyStructBytes(pxStructByte);
  }
  
  pxStructByte->usLength = sLength;
  pxStructByte->pcBytes = (unsigned char*)pvPortMalloc(pxStructByte->usLength*sizeof(unsigned char));
  for(int i = 0; i < pxStructByte->usLength; i++)
  {
    pxStructByte->pcBytes[i] = 0;
  }
}

void vSetBitInLong32(long int *lLongWord, unsigned char usBitNumb, unsigned char usBitValue)
{
  if(usBitValue == 0)
  {
    *lLongWord &= (0xfffffffe << usBitNumb) | (0xffffffff >> sizeof(long int)*8 - usBitNumb);
  }
  else if(usBitValue == 1)
  {
    *lLongWord |= (0x00000001 << usBitNumb);
  }
}

//Copy Struct Of Bytes
void vCopyStructBytes(Bytes *first, Bytes *second)
{
  if(first->usLength == 0)
  {
    return;
  }
  if(second->usLength != 0)
  {
    return;
  }
  
  vCreateStructBytes(second, first->usLength);
  
  for(int i = 0; i < second->usLength; i++)
  {
    second->pcBytes[i] = first->pcBytes[i];
  }
  
}

//Function get data from the recv packet, if its present in packet
void vGetDataFromRecvPack(unsigned char pcRecv_buffer[], int sBuflen, LongWord32 *pxDataMas)
{
  if(sBuflen <= 8)
  {
    return;
  }
  
  vCreateStructDataWordsX32(pxDataMas, (unsigned int)(sBuflen - 8) / 4);
  
  for(int i = 4; i < sBuflen - 4; i++)
  {
    pxDataMas->plWords[i/4 - 1] |= 0x00000000 | pcRecv_buffer[i] << (i % 4) * 8;
  }
}

//Write Pack Length into pack
void vGetAndWritePackLength(Bytes *pxPackWoCheckSum)
{
  pxPackWoCheckSum->pcBytes[0] = (unsigned char)(pxPackWoCheckSum->usLength & 0x00ff);
  pxPackWoCheckSum->pcBytes[1] = (unsigned char)((pxPackWoCheckSum->usLength & 0xff00) >> 4);
}

/*//Calculate CheckSum Packet CheckSum
void vCalcCheckSum(Bytes *pxDataWoCheckSum)
{
  for(int i = 0; i < (pxDataWoCheckSum->usLength / 4) - 1; i++)
  {
    pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 4] |= pxDataWoCheckSum->pcBytes[4 * i + 0];
    pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 3] |= pxDataWoCheckSum->pcBytes[4 * i + 1];
    pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 2] |= pxDataWoCheckSum->pcBytes[4 * i + 2];
    pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 1] |= pxDataWoCheckSum->pcBytes[4 * i + 3];
  }
}

//Function to check and CheckSum from receiver packet
unsigned char cCalcAndCheckCheckSumRecvPack(unsigned char pcRecv_buffer[], int sBuflen)
{
  unsigned char ucCalcCS[4] = {0x00, 0x00, 0x00, 0x00};
  
  for(int i = 0; i < sBuflen; i++)
  {
    if(i < sBuflen - 4)
    {
      ucCalcCS[i % 4] |= pcRecv_buffer[i];
    }
    else
    {
      if(ucCalcCS[i % 4] != pcRecv_buffer[i])
      {
        return 1;
      }
    }
  }
  
  return 0;
}*/

void vCalcCheckSum(Bytes *pxDataWoCheckSum)
{
  
  //old method of calculating checksum
  /*for(int i = 0; i < (pxDataWoCheckSum->usLength / 4) - 1; i++)
  {
    pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 4] |= pxDataWoCheckSum->pcBytes[4 * i + 0];
    pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 3] |= pxDataWoCheckSum->pcBytes[4 * i + 1];
    pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 2] |= pxDataWoCheckSum->pcBytes[4 * i + 2];
    pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 1] |= pxDataWoCheckSum->pcBytes[4 * i + 3];
  }*/
  
  unsigned long int ulCalcCS = 0;
  
  for(int i = 0; i < pxDataWoCheckSum->usLength - 4; i++)
  {
    ulCalcCS = ulCalcCS + (pxDataWoCheckSum->pcBytes[i] * (i + 1));
  }
                           
  pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 4] = ulCalcCS & 0x000000ff;
  pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 3] = (ulCalcCS >> 8) & 0x000000ff;
  pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 2] = (ulCalcCS >> 16) & 0x000000ff;
  pxDataWoCheckSum->pcBytes[pxDataWoCheckSum->usLength - 1] = (ulCalcCS >> 24) & 0x000000ff;
  
}

//Function to check and CheckSum from receiver packet
unsigned char cCalcAndCheckCheckSumRecvPack(unsigned char pcRecv_buffer[], int sBuflen)
{
  unsigned long int ulCalcCS = 0;
  unsigned long int ulRecvCS = 0x00000000 | pcRecv_buffer[sBuflen - 1] << 24 | pcRecv_buffer[sBuflen - 2] << 16 | pcRecv_buffer[sBuflen - 3] << 8 | pcRecv_buffer[sBuflen - 4];
  
  for(int i = 0; i < sBuflen - 4; i++)
  {
    ulCalcCS = ulCalcCS + (pcRecv_buffer[i] * (i + 1));
  }
                           
  if(ulCalcCS != ulRecvCS)
  {
    return 1;
  }
  
  //old method of calculating checksum
  /*unsigned char ucCalcCS[4] = {0x00, 0x00, 0x00, 0x00};
  
  for(int i = 0; i < sBuflen; i++)
  {
    if(i < sBuflen - 4)
    {
      ucCalcCS[i % 4] |= pcRecv_buffer[i];
    }
    else
    {
      if(ucCalcCS[i % 4] != pcRecv_buffer[i])
      {
        return 1;
      }
    }
  }*/
  
  return 0;
}






//Create header from the errcode and opercode
Bytes xCreateHeader(unsigned char sErrCode, unsigned char sOperCode)
{
  Bytes xHeader;
  vInitStructBytes(&xHeader);
  vCreateStructBytes(&xHeader, 4);
  
  xHeader.pcBytes[0] = 0x00;
  xHeader.pcBytes[1] = 0x00;
  xHeader.pcBytes[2] = sOperCode;
  xHeader.pcBytes[3] = sErrCode;
  
  return xHeader;
}

//Function to collect Pack to transmit from header and data
Bytes xCollectPackToTransm(Bytes *pxHeader, Bytes *pxData)
{
  Bytes xPackToTransm;
  vInitStructBytes(&xPackToTransm);
  vCreateStructBytes(&xPackToTransm, pxHeader->usLength + pxData->usLength + 4);
  
  for(int i = 0; i < pxHeader->usLength; i++)
  {
    xPackToTransm.pcBytes[i] = pxHeader->pcBytes[i];
  }
  
  for(int i = 0; i < pxData->usLength; i++)
  {
    xPackToTransm.pcBytes[i + pxHeader->usLength] = pxData->pcBytes[i];
  }
  
  for(int i = xPackToTransm.usLength - 4; i < xPackToTransm.usLength; i++)
  {
    xPackToTransm.pcBytes[i] = 0x00;
  }
    
  vGetAndWritePackLength(&xPackToTransm);
  vCalcCheckSum(&xPackToTransm);
  
  //vDestroyStructBytes(pxHeader);
  vDestroyStructBytes(pxData);
  
  return xPackToTransm;
}

//Function to convert x32 mass of x32 words to bytes
Bytes xConvertWordsToBytes(LongWord32 *pxData)
{
  Bytes xDataBytes;
  vInitStructBytes(&xDataBytes);
  vCreateStructBytes(&xDataBytes, pxData->usLength * 4);
  
  for(int i = 0; i < xDataBytes.usLength; i++)
  {
    xDataBytes.pcBytes[i] = (unsigned char)((pxData->plWords[i / 4] >> ((i % 4) * 8)) & 0x000000ff);
  }
  
  return xDataBytes;
}


//-------------------------------------------------------------------------------------------------

unsigned long int isCheckSum(unsigned long int recv_packet[], unsigned int length)
{
  unsigned long int checkSum = 0x00000000;
  
  for(int i = 0; i < length - 1; i++)
  {
    checkSum |= recv_packet[i];
  }
  
  return checkSum;
}

unsigned long int collectWord32(unsigned char first, unsigned char second, unsigned char third, unsigned char fourth)
{
  unsigned long int word32 = 0x00000000;
  
  word32 |= (fourth << 24) | (third << 16) | (second << 8) | first;
  
  return word32;
}

unsigned char* wordOfError()
{
  unsigned char result[4];
  
  return result;
}

//Get pointer to Bytes to Transmit Queue
Bytes * pGetBytesToTransmQueue(Bytes * pxBytesToTransmQueue)
{
  xSemaphoreTake(xMutexBytesTransmQueue, portMAX_DELAY);
  for(int i = 0; i < QUEUE_TRANSM_QUANT; i++)
  {
    if(xMasToTransmQueue[i].usLength == 0)
    {
      vCopyStructBytes(pxBytesToTransmQueue, &xMasToTransmQueue[i]);
      xSemaphoreGive(xMutexBytesTransmQueue);
      return &xMasToTransmQueue[i];
    }
  }  
  xSemaphoreGive(xMutexBytesTransmQueue);
    
}

//Pushing to queue to transmit thread
void vPushToTransmQueue(Bytes * xDataToTransm)
{
  Bytes * pPointToTransmMas;
  
  pPointToTransmMas = pGetBytesToTransmQueue(xDataToTransm);
  
  if(xDataToTransm->pcBytes[2] == 0xfc)
  {
    xQueueSendToFront(xQueueToTransm, &pPointToTransmMas, portMAX_DELAY);
    return;
  }
  
  if(xQueueSendToBack(xQueueToTransm, &pPointToTransmMas, portMAX_DELAY) == pdPASS)
  {
    unsigned portBASE_TYPE y = uxQueueMessagesWaiting(xQueueToTransm);
    int k = 5;
    int c = (int)y; 
  }
  else
  {
    unsigned portBASE_TYPE y = uxQueueMessagesWaiting(xQueueToTransm);
  }
      
}

//Function to check commands in received data
unsigned char ucCheckCommInRecvData(LongWord32 *xRecvData, unsigned char ucOperCode)
{
  //------------------------------------------0x01--------------------------------------------------
  if(ucOperCode == 0x01)
  {
    if(xRecvData->usLength != 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[0] < 0 || xRecvData->plWords[0] > 255)
    {
      return 1;
    }
  }
  
  //------------------------------------------0x03--------------------------------------------------
  if(ucOperCode == 0x03)
  {
    if(xRecvData->usLength != 3)
    {
      return 1;
    }
    
    if(xRecvData->plWords[0] > 3 || xRecvData->plWords[0] < 1)
    {
      return 1;
    }
      
    if(xRecvData->plWords[1] != 0 && xRecvData->plWords[1] != 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[2] != 0 && xRecvData->plWords[2] != 1)
    {
      return 1;
    }
  }
  
  //------------------------------------------0x04--------------------------------------------------
  else if(ucOperCode == 0x04)
  {
    if(xRecvData->usLength != 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[0] != 0 && xRecvData->plWords[0] != 1 && xRecvData->plWords[0] != 255)
    {
      return 1;
    }
  }
  
  //------------------------------------------0x05--------------------------------------------------
  else if(ucOperCode == 0x05)
  {
    if(xRecvData->usLength != 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[0] < 1 || xRecvData->plWords[0] > 7)
    {
      return 1;
    }
  }
  
  //------------------------------------------0x06--------------------------------------------------
  else if(ucOperCode == 0x06)
  {
    if(xRecvData->usLength != 6)
    {
      return 1;
    }
    
    for(int i = 0; i < xRecvData->usLength; i++)
    {
      if(xRecvData->plWords[i] != 0 && xRecvData->plWords[i] != 1)
      {
        return 1;
      }
    }
  }
  
  //------------------------------------------0x07--------------------------------------------------
  else if(ucOperCode == 0x07)
  {
    if(xRecvData->usLength != 4)
    {
      return 1;
    }
    
    if(xRecvData->plWords[0] < -1 || xRecvData->plWords[0] > 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[1] != 0 || xRecvData->plWords[1] != 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[2] != 0 || xRecvData->plWords[2] != 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[4] < 0 || xRecvData->plWords[4] > 100)
    {
      return 1;
    }
  }
  
  //------------------------------------------0x08--------------------------------------------------
  else if(ucOperCode == 0x08)
  {
    if(xRecvData->usLength != 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[0] > 0x00ffffff)
    {
      return 1;
    }
  }
  
  //------------------------------------------0x09--------------------------------------------------
  else if(ucOperCode == 0x09)
  {
    if(xRecvData->usLength != 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[0] > 0x00ffffff)
    {
      return 1;
    }
  }
  
  //------------------------------------------0x0A--------------------------------------------------
  else if(ucOperCode == 0x0A)
  {
    if(xRecvData->usLength != 1)
    {
      return 1;
    }
    
    if(xRecvData->plWords[0] > 0x00ffffff)
    {
      return 1;
    }
  }
  
  
  //------------------------------------------0x0B----------------------------------------------------
  else if(ucOperCode == 0x0b)
  {
    if(xRecvData->usLength != 5)
    {
      return 1;
    }
    
    for(int i = 0; i < xRecvData->usLength; i++)
    {
      if(xRecvData->plWords[i] != 0 && xRecvData->plWords[i] != 1)
      {
        return 1;
      }
    }
  }
  
  
  return 0;
}


//Initializing char variables
/*void initBytesMas(bytes *structToInit, unsigned char* bytesToInit)
{
  for(int i = 0; i < structToInit->length; i++)
  {
    structToInit->bytes[i] = bytesToInit[i];
  }
}

bytes concatMas(bytes firstPack, bytes secondPack)
{
  bytes resPack;
  
  resPack.length = firstPack.length + secondPack.length;
  
  resPack.bytes = (unsigned char*)pvPortMalloc(resPack.length*sizeof(unsigned char));
  
  for(int i = 0; i < firstPack.length; i++)
  {
    resPack.bytes[i] = firstPack.bytes[i];
  }
  
  for(int i = 0; i < secondPack.length; i++)
  {
    resPack.bytes[firstPack.length + i] = secondPack.bytes[i];
  }
  
  return resPack;
}*/



/*void handleError(int conn, unsigned char codeOfError, unsigned char codeOfOperation)
{
  switch (codeOfError) {
    case 0x02:
      write(conn, (const unsigned char*){codeOfError, codeOfOperation}, (size_t)4);
      break;  
    default:
      break;
      
  }
}*/

unsigned char testConn = 0;

//Task for test connection with PC
void vTestConnPC(void * pvParameters)
{
  Bytes xPacketTestConn;
  int *psConn;
  
  psConn = (int *)pvParameters;
  
  vInitStructBytes(&xPacketTestConn);
  vCreateStructBytes(&xPacketTestConn, 8);
  xPacketTestConn.pcBytes[0] = 0x08;
  xPacketTestConn.pcBytes[1] = 0x00;
  xPacketTestConn.pcBytes[2] = 0xff;
  xPacketTestConn.pcBytes[3] = 0x00;
  xPacketTestConn.pcBytes[4] = 0x05;
  xPacketTestConn.pcBytes[5] = 0x03;
  xPacketTestConn.pcBytes[6] = 0x00;
  xPacketTestConn.pcBytes[7] = 0x00;
  
  while(testConn == 1)
    {
      if(pcInQueue[0xff] > 5)
     {
       vTaskDelay(300);
       continue;
     }
      
      vPushToTransmQueue(&xPacketTestConn);  
      vTaskDelay(500);
    }
  
  vDestroyStructBytes(&xPacketTestConn);
  vTaskDelete(NULL);
}

//Task for send cur coord of detect to PC
//Now only from current coord detector!!!!!!!
void vSendCurCoordCycle(void * pvParameters)
{
  Bytes xPackToTransm, xHeader, xData;
  LongWord32 xCurCoord;
  
  xHeader = xCreateHeader((unsigned char)0x00, (unsigned char)0x04);
  
  vInitStructDataWordsX32(&xCurCoord);
  vCreateStructDataWordsX32(&xCurCoord, 1);
  
  do
  {
    if(pcInQueue[0x04] > 5)
     {
       vTaskDelay(300);
       continue;
     }
    
    xSemaphoreTake(xMutexAccessToPLIS, portMAX_DELAY);
    
    xCurCoord.plWords[0] = getCurrCoordFromDetect();
    
    xSemaphoreGive(xMutexAccessToPLIS);
    
    //xCurCoord.plWords[0] = 2;
    xData = xConvertWordsToBytes(&xCurCoord);
    xPackToTransm = xCollectPackToTransm(&xHeader, &xData);
    
    vPushToTransmQueue(&xPackToTransm);
    
    vDestroyStructBytes(&xData);
    vDestroyStructBytes(&xPackToTransm);
    
    vTaskDelay(200);
    
  }while(ucCoordInCycle == 1);
  
  vDestroyStructDataWors32(&xCurCoord);
  vDestroyStructBytes(&xHeader);
  vDestroyStructBytes(&xData);
  vDestroyStructBytes(&xPackToTransm);
  
  vTaskDelete(NULL);
}

//Task for send current state bits to PC
void vSendCurState(void * pvParameters)
{
  Bytes xPackToTransm, xHeader, xData;
  LongWord32 xCurState;
  
  xHeader = xCreateHeader((unsigned char)0x00, (unsigned char)0x0c);
  
  vInitStructDataWordsX32(&xCurState);
  vCreateStructDataWordsX32(&xCurState, 1);
  xCurState.plWords[0] = 0x00000000;
  
  do
  {
     if(pcInQueue[0x0c] > 5)
     {
       vTaskDelay(300);
       continue;
     }
    
    xSemaphoreTake(xMutexAccessToPLIS, portMAX_DELAY);
    
    vSetBitInLong32(&xCurState.plWords[0], 0, getStateBitADCReq() );
    vSetBitInLong32(&xCurState.plWords[0], 1, getStateBitStopReq() );
    vSetBitInLong32(&xCurState.plWords[0], 2, getStateBitSlowReq() );
    vSetBitInLong32(&xCurState.plWords[0], 3, getStateBitVd0() );
    vSetBitInLong32(&xCurState.plWords[0], 4, getStateBitVd1() );
    vSetBitInLong32(&xCurState.plWords[0], 5, getStateBitPermZeroReset() );
    
    vSetBitInLong32(&xCurState.plWords[0], 8, getStateBitEVMOrMest() );
    vSetBitInLong32(&xCurState.plWords[0], 9, getStateBitATVReady() );
    vSetBitInLong32(&xCurState.plWords[0], 10, getStateBitATVCrash() );
    vSetBitInLong32(&xCurState.plWords[0], 11, getStateBitATVBlocking() );
    vSetBitInLong32(&xCurState.plWords[0], 12, getStateBitStopR() );
    vSetBitInLong32(&xCurState.plWords[0], 13, getStateBitStopL() );
    vSetBitInLong32(&xCurState.plWords[0], 14, getStateBitDriveR() );
    vSetBitInLong32(&xCurState.plWords[0], 15, getStateBitDriveL() );
    
    vSetBitInLong32(&xCurState.plWords[0], 16, getStateBitSlowSpeed() );
    vSetBitInLong32(&xCurState.plWords[0], 17, getStateBitEngUgolMesta() );
    vSetBitInLong32(&xCurState.plWords[0], 18, getStateBitEngPeremesch() );
    vSetBitInLong32(&xCurState.plWords[0], 19, getStateBitEngAzimut() );
    vSetBitInLong32(&xCurState.plWords[0], 20, getStateBitEngFokal() );
    vSetBitInLong32(&xCurState.plWords[0], 21, getStateBitEngPodyom() );
    vSetBitInLong32(&xCurState.plWords[0], 22, getStateEngColona() );
    vSetBitInLong32(&xCurState.plWords[0], 23, getStateEngKaretka() );
    
    xSemaphoreGive(xMutexAccessToPLIS);

    xData = xConvertWordsToBytes(&xCurState);
    xPackToTransm = xCollectPackToTransm(&xHeader, &xData);
    
    vPushToTransmQueue(&xPackToTransm);
    
    vDestroyStructBytes(&xData);
    vDestroyStructBytes(&xPackToTransm);
    
    vTaskDelay(500);
    
  }while(ucStateInCycle == 1);
  
  vDestroyStructDataWors32(&xCurState);
  vDestroyStructBytes(&xHeader);
  vDestroyStructBytes(&xData);
  vDestroyStructBytes(&xPackToTransm);
  
  vTaskDelete(NULL);
}


//Function to transmit pack
void vTransmPack(void * pvParameters)
{
  Bytes * xPacketToTransm;
  int *psConn;
  psConn = (int *)pvParameters;
  /*for(int i = 0; i < QUEUE_TRANSM_QUANT; i++)
  {
    vInitStructBytes(&xMasToTransmQueue[i]);
  }*/
  
  for(int i = 0; i < 256; i++)
  {
    pcInQueue[i] = 0;
  }
  
  while(1)
  {
    if(xQueueReceive(xQueueToTransm, &xPacketToTransm, portMAX_DELAY) == pdPASS)
    {
      xSemaphoreTake(xTransmitSemaphore, portMAX_DELAY);
      xSemaphoreTake(xMutexBytesTransmQueue, portMAX_DELAY);
      write(*psConn, (const unsigned char*)xPacketToTransm->pcBytes, (size_t)xPacketToTransm->usLength);
      
      if(pcInQueue[xPacketToTransm->pcBytes[2]] != 0)
      {
        pcInQueue[xPacketToTransm->pcBytes[2]] = pcInQueue[xPacketToTransm->pcBytes[2]] - 1;
      }
      
      vDestroyStructBytes(xPacketToTransm);
      xSemaphoreGive(xMutexBytesTransmQueue);
    }
    else
    {
      int x = 15;
    }
  }
  
  vTaskDelete(NULL);
}

void board_server_serve(int conn) 
{
  
  //variables
  int buflen= 1500;
  int ret;
  struct fs_file * file;
  unsigned char recv_buffer[1500];
  
  Bytes xPctAnswHeader, xPctAnswData, xPctAnswToTransm;
  LongWord32 xRecvData;
  
  vInitStructBytes(&xPctAnswHeader);
  vCreateStructBytes(&xPctAnswHeader, 4);
  vInitStructBytes(&xPctAnswData);
  vInitStructBytes(&xPctAnswToTransm);
  vInitStructDataWordsX32(&xRecvData);
  
  xTaskCreate(vTransmPack, "transmit", configMINIMAL_STACK_SIZE, (void*)&conn, WEBSERVER_THREAD_PRIO + 3, NULL);
  
  //-----------------------------------handle request---------------------------------------
  while(1)
  {
    /* Read in the request */
    ret = read(conn, (void**)&recv_buffer, buflen);
    if(ret <= 0) continue;

    if(ret % 4 != 0)
    {
      unsigned char errPct[4] = {0x00, 0x00, recv_buffer[2], 0x03};
      write(conn, (const unsigned char*)errPct, (size_t)4);
      continue;
    }
  
    if(cCalcAndCheckCheckSumRecvPack(recv_buffer, ret) != 0)
    {
      unsigned char errPct[4] = {0x00, 0x00, recv_buffer[2], 0x02};
      write(conn, (const unsigned char*)errPct, (size_t)4);
      continue;
    }
  
    if(ret != ((recv_buffer[1] << 8) | recv_buffer[0]) )
    {
      unsigned char errPct[4] = {0x00, 0x00, recv_buffer[2], 0x03};
      write(conn, (const unsigned char*)errPct, (size_t)4);
      continue;
    }
  
    if(ret >= 8)
    {
      
    }
    
    
  
    switch (recv_buffer[2]){ 
      case 0x01:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = recv_buffer[2];
        xPctAnswHeader.pcBytes[3] = 0x00;
        
        vCreateStructBytes(&xPctAnswData, 4);
        if(ret > 8)
        {
          xPctAnswData.pcBytes[0] = dataExchangeTest(recv_buffer[4]);  
        }
        else
        {
          xPctAnswData.pcBytes[0] = dataExchangeTest((unsigned char)0x00);
        }
        xPctAnswData.pcBytes[1] = 0x00;
        xPctAnswData.pcBytes[2] = 0x00;
        xPctAnswData.pcBytes[3] = 0x00;
        break;
      case 0x02:
        vCreateStructBytes(&xPctAnswData, 4);
        for(int i = 0; i < 20; i++)
        {
          xPctAnswHeader.pcBytes[0] = 0x00;
          xPctAnswHeader.pcBytes[1] = 0x00;
          xPctAnswHeader.pcBytes[2] = 0x02;
          xPctAnswHeader.pcBytes[3] = 0x00;
          xPctAnswData.pcBytes[0] = (unsigned char)i;
          xPctAnswData.pcBytes[1] = 0x00;
          xPctAnswData.pcBytes[2] = 0x00;
          xPctAnswData.pcBytes[3] = 0x00;

          xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
          vPushToTransmQueue(&xPctAnswToTransm);
          vTaskDelay(400 * (i + 1));
        }
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        continue;
        break;
      case 0x03:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = 0x03;
        
        vGetDataFromRecvPack(recv_buffer, ret, &xRecvData);
        if(ucCheckCommInRecvData(&xRecvData, 0x03) == 0)
        {
          setCurCoordDet((unsigned char)(xRecvData.plWords[0] - 1), (unsigned char)xRecvData.plWords[1], (unsigned char)xRecvData.plWords[2]);
          
          if( ((getStateBitVd0() << 1) | getStateBitVd1()) != (unsigned char)(xRecvData.plWords[0] - 1))
          {
            xPctAnswHeader.pcBytes[3] = 0x05;
          }
          else
          {
            xPctAnswHeader.pcBytes[3] = 0x00; 
          }
        }
        else
        {
          xPctAnswHeader.pcBytes[3] = 0x01;
        }
        
        xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
        vPushToTransmQueue(&xPctAnswToTransm);
          
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        vDestroyStructDataWors32(&xRecvData);
        continue;
        break;
      case 0x04:
        vGetDataFromRecvPack(recv_buffer, ret, &xRecvData);
        if(ucCheckCommInRecvData(&xRecvData, 0x04) == 0)
        {
          if(xRecvData.plWords[0] != 0x00)
          {
            if(ucCoordInCycle != 1)
            {
              if(xRecvData.plWords[0] == 0xff)
              {
                ucCoordInCycle = 1;
              }
              else if(xRecvData.plWords[0] == 0x01)
              {
                ucCoordInCycle = 0;
              }
            
              xTaskCreate(vSendCurCoordCycle, "sendCurCoord", configMINIMAL_STACK_SIZE, NULL, WEBSERVER_THREAD_PRIO, NULL);
            }
          }
          else if(xRecvData.plWords[0] == 0x00)
          {
            ucCoordInCycle = 0;
          }
        }
        
        vDestroyStructDataWors32(&xRecvData);
        continue;
        break;
      case 0x05:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = 0x05;
        
        vGetDataFromRecvPack(recv_buffer, ret, &xRecvData);
        if(ucCheckCommInRecvData(&xRecvData, 0x05) == 0)
        {
          switchEngine((unsigned char)(xRecvData.plWords[0] - 1));
          
          if(getActiveEngineFromPLIS() != (unsigned char)(xRecvData.plWords[0] - 1))
          {
            xPctAnswHeader.pcBytes[3] = 0x06;
          }
          else
          {
            xPctAnswHeader.pcBytes[3] = 0x00;
          }
        }
        else
        {
          xPctAnswHeader.pcBytes[3] = 0x01;
        }
        
        xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
        vPushToTransmQueue(&xPctAnswToTransm);
          
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        vDestroyStructDataWors32(&xRecvData);
        continue;
        break;
      case 0x06:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = 0x06;
        
        vGetDataFromRecvPack(recv_buffer, ret, &xRecvData);
        if(ucCheckCommInRecvData(&xRecvData, 0x06) == 0)
        {
          setComparatorsOptions((unsigned char)xRecvData.plWords[0], (unsigned char)xRecvData.plWords[3], (unsigned char)xRecvData.plWords[1], (unsigned char)xRecvData.plWords[4], (unsigned char)xRecvData.plWords[2], (unsigned char)xRecvData.plWords[5]);
        }
        else
        {
          xPctAnswHeader.pcBytes[3] = 0x01;
        }
        
        xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
        vPushToTransmQueue(&xPctAnswToTransm);
          
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        vDestroyStructDataWors32(&xRecvData);
        continue;
        break;
      case 0x07:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = 0x07;
        
        vGetDataFromRecvPack(recv_buffer, ret, &xRecvData);
        if(ucCheckCommInRecvData(&xRecvData, 0x07) == 0)
        {
          slowSpeedSwitch((unsigned char)xRecvData.plWords[1]);
          switchBrakingMode((unsigned char)xRecvData.plWords[2]);
          
          unsigned char usSpeedCode;
          
          if((unsigned char)xRecvData.plWords[3] == 100)
          {
            usSpeedCode = (unsigned char)0x00;
          }
          else if((unsigned char)xRecvData.plWords[3] == 75)
          {
            usSpeedCode = (unsigned char)0x01;
          }
          else if((unsigned char)xRecvData.plWords[3] == 50)
          {
            usSpeedCode = (unsigned char)0x10;
          }
          else if((unsigned char)xRecvData.plWords[3] == 25)
          {
            usSpeedCode = (unsigned char)0x11;
          }
          
          switchEngineSpeed(usSpeedCode);
          motionDirectOfEngine((signed char)xRecvData.plWords[0]);
          
          if( !(xRecvData.plWords[0] == -1 && getStateBitDriveR() == 1) && !(xRecvData.plWords[0] == 1 && getStateBitStopL() == 1) && !(xRecvData.plWords[0] == 0 && getStateBitStopL() == 0 && getStateBitDriveR() == 0))
          {
            xPctAnswHeader.pcBytes[3] = 0x07;
          }
          
          if(!(xRecvData.plWords[1] == 1 && getStateBitSlowSpeed() == 1) && !(xRecvData.plWords[1] == 0 && getStateBitSlowSpeed() == 0))
          {
            xPctAnswHeader.pcBytes[3] = 0x07;
          }
          
        }
        else
        {
          xPctAnswHeader.pcBytes[3] = 0x01;
        }
        
        xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
        vPushToTransmQueue(&xPctAnswToTransm);
          
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        vDestroyStructDataWors32(&xRecvData);
        continue;
        break;
      case 0x08:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = 0x08;
        
        vGetDataFromRecvPack(recv_buffer, ret, &xRecvData);
        if(ucCheckCommInRecvData(&xRecvData, 0x08) == 0)
        {
          writeCoordToADCComp(xRecvData.plWords[0]);
        }
        else
        {
          xPctAnswHeader.pcBytes[3] = 0x01;
        }
        
        xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
        vPushToTransmQueue(&xPctAnswToTransm);
          
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        vDestroyStructDataWors32(&xRecvData);
        continue;
        break;
      case 0x09:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = 0x09;
        
        vGetDataFromRecvPack(recv_buffer, ret, &xRecvData);
        if(ucCheckCommInRecvData(&xRecvData, 0x09) == 0)
        {
          writeCoordToMotorComp(xRecvData.plWords[0]);
        }
        else
        {
          xPctAnswHeader.pcBytes[3] = 0x01;
        }
        
        xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
        vPushToTransmQueue(&xPctAnswToTransm);
          
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        vDestroyStructDataWors32(&xRecvData);
        continue;
        break;
      case 0x0a:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = 0x0a;
        
        vGetDataFromRecvPack(recv_buffer, ret, &xRecvData);
        if(ucCheckCommInRecvData(&xRecvData, 0x0a) == 0)
        {
          writeCoordToSlowSpeedComp(xRecvData.plWords[0]);
        }
        else
        {
          xPctAnswHeader.pcBytes[3] = 0x01;
        }
        
        xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
        vPushToTransmQueue(&xPctAnswToTransm);
          
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        vDestroyStructDataWors32(&xRecvData);
        continue;
        break;
      case 0x0b:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = 0x0b;
          
        vGetDataFromRecvPack(recv_buffer, ret, &xRecvData);
        if(ucCheckCommInRecvData(&xRecvData, 0x0b) == 0)
        {
          for(int i = 0; i < 5; i++)
          {
            if(xRecvData.plWords[i] == 0)
            {
              setVV88MFromDKS((unsigned char)(i + 1));
            }
            else if(xRecvData.plWords[i] == 1)
            {
              setVV88MFromZond((unsigned char)(i + 1));
            }
          }
        }
        else
        {
          xPctAnswHeader.pcBytes[3] = 0x01;
        }
        
        xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
        vPushToTransmQueue(&xPctAnswToTransm);
          
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        vDestroyStructDataWors32(&xRecvData);
        continue;
        break;
      case 0x0c:
        
        if(ucStateInCycle != 1)
        {
          xTaskCreate(vSendCurState, "sendCurState", configMINIMAL_STACK_SIZE, NULL, WEBSERVER_THREAD_PRIO, NULL);
          ucStateInCycle = 1;
        }
        
        continue;
        break;
      case 0xfc:
        xPctAnswHeader.pcBytes[0] = 0x00;
        xPctAnswHeader.pcBytes[1] = 0x00;
        xPctAnswHeader.pcBytes[2] = 0xfc;
        xPctAnswHeader.pcBytes[3] = 0x00;
        
        xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
        vPushToTransmQueue(&xPctAnswToTransm);
          
        vDestroyStructBytes(&xPctAnswData);
        vDestroyStructBytes(&xPctAnswToTransm);
        vDestroyStructDataWors32(&xRecvData);
        
        
        continue;
        break;
      case 0xfe:
        xSemaphoreGive(xTransmitSemaphore);
        continue;
        break;
      case 0xff:
        /*constrOFBytesStruct(&pctWoCheckSum, 4);
        pctWoCheckSum.bytes[0] = 0x00;
        pctWoCheckSum.bytes[1] = 0x00;
        pctWoCheckSum.bytes[2] = 0xff;
        pctWoCheckSum.bytes[3] = 0x00;*/
        
        if(testConn == 0)
        {
          testConn = 1;
          xTaskCreate(vTestConnPC, "testConnPC", configMINIMAL_STACK_SIZE, (void*)&conn, WEBSERVER_THREAD_PRIO, NULL);
        }
        continue;
        break;
      default:
        continue;
        break;
    }
    
    xPctAnswToTransm = xCollectPackToTransm(&xPctAnswHeader, &xPctAnswData);
    vPushToTransmQueue(&xPctAnswToTransm);

    vDestroyStructBytes(&xPctAnswData);
    vDestroyStructBytes(&xPctAnswToTransm);
    //write(conn, (const unsigned char*)xPctAnswToTransm.pcBytes, (size_t)xPctAnswToTransm.usLength);
      
    //resetWriteDataToPlis();

  
  
  }
  
  vDestroyStructBytes(&xPctAnswHeader);
  vDestroyStructBytes(&xPctAnswData);
  vDestroyStructBytes(&xPctAnswToTransm);
  
  close(conn);
  return;
}

/**
  * @brief  http server thread 
  * @param arg: pointer on argument(not used here) 
  * @retval None
  */
static void board_server_socket_thread(void *pvParameters)
{
  int sock, newconn, size;
  struct sockaddr_in address, remotehost;

 /* create a TCP socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
  {
    printf("can not create socket");
    return;
  }
  
  /* bind to port 12345 at any interface */
  address.sin_family = AF_INET;
  address.sin_port = htons(12345);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
  {
    printf("can not bind socket");
    return;
  }
  
  /* listen for incoming connections (TCP listen backlog = 5) */
  listen(sock, 5);
  
  size = sizeof(remotehost);
  
  while (1) 
  {
    newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    board_server_serve(newconn);
  }
}

/**
  * @brief  Initialize the board server (start its thread) 
  * @param  none
  * @retval None
  */
void board_server_socket_init()
{
  //xTaskCreate(board_server_socket_thread, "board_server", DEFAULT_THREAD_STACKSIZE * 2, NULL, WEBSERVER_THREAD_PRIO, NULL);
  sys_thread_new("b_serv", board_server_socket_thread, NULL, DEFAULT_THREAD_STACKSIZE * 2, WEBSERVER_THREAD_PRIO );
}






/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
