/* main.cpp
 *
 * Created on: 7. 3. 2022
 * Author: Martin Danek, martin@embedblog.eu
 * baud 115200
 */

#include "main.h"

#include <stdio.h>

extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart2;

int main()
{
	HAL_Init();
	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();

	MX_USART2_UART_Init();
	MX_I2C2_Init();

	printf("Device started!\n");

	while (true)
	{
		HAL_GPIO_TogglePin(REDLED_GPIO_Port, REDLED_Pin);
		HAL_Delay(500);

		printf("tick1\n");
		printf("tick2\n");
	}
}

extern "C" int _write(int file, char* ptr, int len)
{
	/*while (HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_TX)
		HAL_Delay(1);

	HAL_UART_Transmit_DMA(&huart2, (uint8_t*)ptr, len);*/

	HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, 100);

	return len;
}

