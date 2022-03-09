/* main.cpp
 *
 * Created on: 7. 3. 2022
 * Author: Martin Danek, martin@embedblog.eu
 * baud 115200
 */

#include "main.h"

#include "comport.h"
#include "proj_conf.h"
#include "decoder.h"

#include "eeprom24.h"
#include "custom_assert.h"

extern I2C_HandleTypeDef hi2c2;
Eeprom24_08 eeprom{&hi2c2};

volatile Mode mode;
volatile bool isSendingRawData;
volatile bool isEepromSaveRequested;
volatile bool isReadoutRequested;

int main()
{
	HAL_Init();
	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_TIM16_Init();
	MX_I2C2_Init();

	//startup blink
	for (uint_fast8_t i = 0; i < 6; i++)
	{
		HAL_GPIO_TogglePin(REDLED_GPIO_Port, REDLED_Pin);
		HAL_Delay(250);
	}

	HAL_Delay(500);
	comport::init();

	//init eeprom & load data
	ASSERT(eeprom.init());
	if (eeprom.readByte(EEPROM_VALID_ADDR) == EEPROM_VALID_FLAG)
	{
		uint8_t data = eeprom.readByte(EEPROM_CONF_ADDR);
		isSendingRawData = data >> 7;
		mode = (Mode)(data & 0b111);
		printf("Valid settings loaded from EEPROM\n");
	}

	uint32_t lastDataTick = 0;
	while (true)
	{
		if ((mode == Mode::SEND_1HZ) && (HAL_GetTick() - lastDataTick >= 1000))
			isReadoutRequested = true;
		else if ((mode == Mode::SEND_5HZ) && (HAL_GetTick() - lastDataTick >= 200))
			isReadoutRequested = true;

		if (isReadoutRequested)
		{
			if (decoder::receiveMessage())
			{
				HAL_GPIO_WritePin(REDLED_GPIO_Port, REDLED_Pin, GPIO_PIN_RESET);
				lastDataTick = HAL_GetTick();
				isReadoutRequested = false;
				decoder::processMessage();
				HAL_Delay(50);
				HAL_GPIO_WritePin(REDLED_GPIO_Port, REDLED_Pin, GPIO_PIN_SET);
			}
		}

		if (isEepromSaveRequested)
		{
			isEepromSaveRequested = false;
			eeprom.writeByte(EEPROM_VALID_ADDR, EEPROM_VALID_FLAG);
			eeprom.waitForReady();
			eeprom.writeByte(EEPROM_CONF_ADDR, (uint8_t)((uint8_t)mode | ((uint8_t)isSendingRawData << 7)));
		}
	}
}



