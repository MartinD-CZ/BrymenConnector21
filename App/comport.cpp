/* comport.cpp
 *
 * Created on: Mar 7, 2022
 * Author: Martin Danek, martin@embedblog.eu
 */

#include "comport.h"

#include "hal_inc.h"
#include "main.h"

extern UART_HandleTypeDef huart2;

static uint8_t rxData[16];

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart->Instance == huart2.Instance)
	{


		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rxData, sizeof(rxData));
	}
}

void comport::init()
{
	MX_USART2_UART_Init();
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rxData, sizeof(rxData));
	printf("Starting device...\n");
}

extern "C" int _write(int file, char* ptr, int len)
{
	auto status = HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, 100);

	if (status == HAL_OK)
		return len;
	else
		return 0;
}

void custom_assert(const char* file, uint32_t line)
{
	printf("ASSERT FAILED! File %s, line %u\n", file, (unsigned int)line);

	#if defined(DEBUG)
	__asm("bkpt 0");		//breakpoint instruction
	#endif
}

