/* main.cpp
 *
 * Created on: 7. 3. 2022
 * Author: Martin Danek, martin@embedblog.eu
 * baud 115200
 */

#include "main.h"

#include "comport.h"

#include "eeprom24.h"

#include <stdio.h>

extern I2C_HandleTypeDef hi2c2;

Eeprom24_08 eeprom{&hi2c2};

int main()
{
	HAL_Init();
	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_TIM16_Init();
	MX_I2C2_Init();
	comport::init();

	eeprom.init();

	while (true)
	{
		HAL_GPIO_TogglePin(REDLED_GPIO_Port, REDLED_Pin);
		HAL_Delay(500);

		//printf("tick1\n");
		//printf("tick2\n");
	}
}



