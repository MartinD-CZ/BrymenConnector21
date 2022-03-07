/* main.cpp
 *
 * Created on: 7. 3. 2022
 * Author: Martin Danek, martin@embedblog.eu
 * baud 115200
 */

#include "main.h"

#include "eeprom24.h"

#include <stdio.h>

extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart2;

Eeprom24_08 eeprom{&hi2c2};

int main()
{
	HAL_Init();
	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();

	MX_USART2_UART_Init();
	MX_I2C2_Init();

	printf("Initializing EEPROM...\n");
	if (eeprom.init())
		printf("OK\n");
	else
		printf("Fail!");

	uint8_t tx[] = {0xDE, 0xAD, 0xBE, 0xEF};
	eeprom.writePage(500, tx, sizeof(tx));

	HAL_Delay(500);

	uint8_t rx[sizeof(tx)];
	eeprom.readPage(500, rx, sizeof(tx));

	while (true)
	{
		HAL_GPIO_TogglePin(REDLED_GPIO_Port, REDLED_Pin);
		HAL_Delay(500);

		//printf("tick1\n");
		//printf("tick2\n");
	}
}

extern "C" int _write(int file, char* ptr, int len)
{
	auto status = HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, 100);

	if (status == HAL_OK)
		return len;
	else
		return 0;
}

