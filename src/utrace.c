/*=======================================================================================*
 * @file    utrace.c
 * @author  Damian Pala
 * @date    03-03-2017
 * @brief   This file contains all implementations for U-Trace module.
 *======================================================================================*/

/**
 * @addtogroup U-Trace Description
 * @{
 * @brief Module for trace via UART with DMA.
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/
/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include "cmsis_device.h"

/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define UTRACE_TMP_BUFFER_SIZE                          256
#define DMAx_STREAM                                     DMA1_Stream6
#define TRANSFER_TIMEOUT                                1000000

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/

/*------------------------------- STRUCT AND UNIONS ------------------------------------*/

/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/
static char buffer[UTRACE_TMP_BUFFER_SIZE];

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/
void ConfigurePins(void);
void UART_Init(void);
void DMA_Initialization(void);
void WaitForBufferEmpty(void);

/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
void UTRACE_Init(void)
{
  ConfigurePins();
  UART_Init();
  DMA_Initialization();
}

void UTRACE_Write(const char* buffer, size_t nbyte)
{
  /* Set DMA buffer */
  DMAx_STREAM->M0AR = (uint32_t)buffer;
  /* Set DMA buffer size */
  DMAx_STREAM->NDTR = (uint32_t)nbyte;
  /* Enable DMA and start data transfer */
  DMAx_STREAM->CR |= (uint32_t)DMA_SxCR_EN;
}

void UTRACE_Printf(const char* format, ...)
{
  int ret;
  va_list ap;

  va_start (ap, format);

  WaitForBufferEmpty();
  ret = vsnprintf(buffer, sizeof(buffer), format, ap);

  if (ret > 0)
  {
    UTRACE_Write(buffer, (size_t)ret);
  }

  va_end (ap);
}

void UTRACE_Puts(const char *str)
{
  WaitForBufferEmpty();
  UTRACE_Write(str, strlen(str));
}

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/
void ConfigurePins(void)
{
  GPIO_InitTypeDef l_GPIO_InitStruct;
  l_GPIO_InitStruct.GPIO_Mode                   = GPIO_Mode_AF;
  l_GPIO_InitStruct.GPIO_OType                  = GPIO_OType_PP;
  l_GPIO_InitStruct.GPIO_Pin                    = GPIO_Pin_2;
  l_GPIO_InitStruct.GPIO_PuPd                   = GPIO_PuPd_UP;
  l_GPIO_InitStruct.GPIO_Speed                  = GPIO_High_Speed;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);

  GPIO_Init(GPIOA, &l_GPIO_InitStruct);
}

void UART_Init(void)
{
  USART_InitTypeDef l_UART_InitStruct;

  l_UART_InitStruct.USART_BaudRate              = 115200;
  l_UART_InitStruct.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
  l_UART_InitStruct.USART_Mode                  = USART_Mode_Tx;
  l_UART_InitStruct.USART_Parity                = USART_Parity_No;
  l_UART_InitStruct.USART_StopBits              = USART_StopBits_1;
  l_UART_InitStruct.USART_WordLength            = USART_WordLength_8b;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  USART_Init(USART2, &l_UART_InitStruct);
  USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
  USART_Cmd(USART2, ENABLE);
}

void DMA_Initialization(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  DMA_DeInit(DMAx_STREAM);

  /* Config of DMAC */
  DMA_InitStructure.DMA_Channel           = DMA_Channel_4;
  DMA_InitStructure.DMA_BufferSize        = 1;
  DMA_InitStructure.DMA_DIR               = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_FIFOMode          = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold     = 0;
  DMA_InitStructure.DMA_MemoryBurst       = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst   = DMA_PeripheralBurst_Single;
  DMA_InitStructure.DMA_Mode              = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority          = DMA_Priority_Medium;

  /* Config of memory */
  DMA_InitStructure.DMA_Memory0BaseAddr    = (uint32_t)buffer;
  DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
  DMA_Init(DMAx_STREAM, &DMA_InitStructure);
}

void WaitForBufferEmpty(void)
{
  uint32_t timeout = TRANSFER_TIMEOUT;
  static bool IsAfterFirstCall = false;

  if (IsAfterFirstCall)
  {
    while( (RESET == DMA_GetFlagStatus(DMAx_STREAM, DMA_FLAG_TCIF6)) && (timeout > 0) )
    {
      timeout--;
    }
    DMA_ClearFlag(DMAx_STREAM, DMA_FLAG_TCIF6);
  }
  else
  {
    IsAfterFirstCall = true;
  }
}

/**
 * @}
 */
