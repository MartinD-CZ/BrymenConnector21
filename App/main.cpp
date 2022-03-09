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

bool isSendingRawData;
Mode mode;			//current operating mode
Request request;	//requests from serial console

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
	bool isReadoutRequested = false;
	while (true)
	{
		if ((mode == Mode::SEND_1HZ) && (HAL_GetTick() - lastDataTick >= 1000))
			isReadoutRequested = true;
		else if (mode == Mode::SEND_5HZ)
			isReadoutRequested = true;

		if (isReadoutRequested)
		{
			uint32_t measurementStart = HAL_GetTick();
			if (decoder::receiveMessage())
			{
				HAL_GPIO_WritePin(REDLED_GPIO_Port, REDLED_Pin, GPIO_PIN_RESET);
				lastDataTick = measurementStart;
				isReadoutRequested = false;
				decoder::processMessage();
				HAL_Delay(50);
				HAL_GPIO_WritePin(REDLED_GPIO_Port, REDLED_Pin, GPIO_PIN_SET);
			}
		}

		if (request != Request::NONE)
		{
			switch(request)
			{
				case Request::SET_MODE_5HZ:
					mode = Mode::SEND_5HZ;
					printf("Setting full datarate (cca 5 Hz)\n");
					break;
				case Request::SET_MODE_1HZ:
					mode = Mode::SEND_1HZ;
					printf("Setting datarate to 1 Hz\n");
					break;
				case Request::SET_MODE_STOP:
					mode = Mode::STOP;
					printf("Stopping data acquisition\n");
					break;
				case Request::REQUEST_DATA:
					mode = Mode::STOP;
					isReadoutRequested = true;
					printf("Getting single sample\n");
					break;
				case Request::SET_RAW:
					isSendingRawData = !isSendingRawData;
					printf("Printing %s values\n", isSendingRawData ? "raw" : "decoded");
					break;
				case Request::SAVE_EEPROM:
					eeprom.writeByte(EEPROM_VALID_ADDR, EEPROM_VALID_FLAG);
					eeprom.waitForReady();
					eeprom.writeByte(EEPROM_CONF_ADDR, (uint8_t)((uint8_t)mode | ((uint8_t)isSendingRawData << 7)));
					printf("Current settings saved to EEPROM\n");
					break;
				case Request::GET_HELP:
					comport::printHelp();
					break;
				default: break;
			}

			request = Request::NONE;
		}
	}
}



