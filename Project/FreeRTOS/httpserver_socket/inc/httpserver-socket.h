/**
  ******************************************************************************
  * @file    httpserver-socket.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011
  * @brief   header file for httpserver-socket.c
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HTTPSERVER_SOCKET_H__
#define __HTTPSERVER_SOCKET_H__

#define QUEUE_TRANSM_QUANT 100

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct massOfBytes{
  unsigned int usLength;
  unsigned char* pcBytes;
} Bytes;

typedef struct massOfWordLong32{
  unsigned int usLength;
  long int* plWords;
} LongWord32;

typedef struct
{
  unsigned int usLength;
  float *pfWords;
}Floats;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void http_server_socket_init(void);
void board_server_socket_init(void);
void DynWebPage(int conn);

void vInitStructBytes(Bytes *pxStructByte);


#endif /* __HTTPSERVER_SOCKET_H__ */






/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
