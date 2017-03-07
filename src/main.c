#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "cmsis_device.h"
#include "utrace.h"

#define GPIOx(PORT_NUMBER)            ((GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(PORT_NUMBER)))
#define PIN_MASK(PIN)                 (1 << (PIN))
#define RCC_PORT_MASKx(PORT_NUMBER)   (RCC_AHB1ENR_GPIOAEN << (PORT_NUMBER))

#define LED_PORT            0
#define LED_PIN             5
#define LED_ON              GPIO_ResetBits(GPIOx(LED_PORT), PIN_MASK(LED_PIN))
#define LED_OFF             GPIO_SetBits(GPIOx(LED_PORT), PIN_MASK(LED_PIN))
#define LED_TOGGLE          GPIO_ToggleBits(GPIOx(LED_PORT), PIN_MASK(LED_PIN))

void LedPortInit(void)
{
  GPIO_InitTypeDef l_GPIO_InitStruct;

  GPIO_StructInit(&l_GPIO_InitStruct);
  l_GPIO_InitStruct.GPIO_Mode                   = GPIO_Mode_OUT;
  l_GPIO_InitStruct.GPIO_OType                  = GPIO_OType_PP;
  l_GPIO_InitStruct.GPIO_Pin                    = PIN_MASK(LED_PIN);
  l_GPIO_InitStruct.GPIO_PuPd                   = GPIO_PuPd_NOPULL;
  l_GPIO_InitStruct.GPIO_Speed                  = GPIO_High_Speed;

  RCC_AHB1PeriphClockCmd(RCC_PORT_MASKx(LED_PORT), ENABLE);

  GPIO_Init(GPIOx(LED_PORT), &l_GPIO_InitStruct);
}

int main(void)
{
  UTRACE_Init();

  LedPortInit();

  trace_puts("Hello ARM World!");

  trace_printf("System clock: %u Hz\n", SystemCoreClock);

  for (uint8_t i = 0; i < 20; i++)
  {
    UTRACE_Printf("Works great %d\r\n", i);
//    UTRACE_Puts("Puts works OK \r\n");
  }

  uint32_t seconds = 0;

  while (1)
	{
    LED_TOGGLE;

	  ++seconds;

	  trace_printf("Second %u\n", seconds);
	  for (uint32_t i = 0; i < 16000000; i++);
	}
}
