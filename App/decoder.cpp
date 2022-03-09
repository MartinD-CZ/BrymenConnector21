#include "decoder.h"

#include "main.h"
#include "proj_conf.h"

#include "hal_inc.h"

uint8_t data[20];
extern volatile bool isSendingRawData;

extern TIM_HandleTypeDef htim16;

void delay_us(uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim16, 0);
	while (__HAL_TIM_GET_COUNTER(&htim16) <= us);
}

uint8_t receiveByte(void)
{
	uint8_t temp = 0;

	for (uint_fast8_t i = 0; i < 8; i ++)
	{
		HAL_GPIO_WritePin(IRLED_GPIO_Port, IRLED_Pin, GPIO_PIN_RESET);
		delay_us(CLOCK_DELAY_US);
		HAL_GPIO_WritePin(IRLED_GPIO_Port, IRLED_Pin, GPIO_PIN_SET);

		delay_us(CLOCK_DELAY_US / 2);
		if (HAL_GPIO_ReadPin(DIN_GPIO_Port, DIN_Pin))
			temp |= (1 << i);
		delay_us(CLOCK_DELAY_US / 2);
	}

	return temp;
}

bool decoder::receiveMessage(void)
{
	HAL_GPIO_WritePin(IRLED_GPIO_Port, IRLED_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(IRLED_GPIO_Port, IRLED_Pin, GPIO_PIN_SET);

	uint32_t start = HAL_GetTick();
	while (HAL_GPIO_ReadPin(DIN_GPIO_Port, DIN_Pin))
	{
		if (HAL_GetTick() - start > 500)
			return false;
	}

	__HAL_TIM_ENABLE(&htim16);
	for (uint8_t i = 0; i < 20; i ++)
		data[i] = receiveByte();
	__HAL_TIM_DISABLE(&htim16);

	return 1;
}

void decoder::processMessage(void)
{
	if (isSendingRawData)
	{
		printf("RAW DATA: ");

		for (uint8_t i = 0; i < 20; i++)
			printf("%02X ", data[i]);

		printf("\n");
		return;
	}

	//decode the output
	if (data[0] & 0x01)
		printf("AUTO ");
	if ((data[0] >> 7) & 0x01)
		printf("AVG ");
	if ((data[0] >> 6) & 0x01)
		printf("MIN ");
	if ((data[0] >> 5) & 0x01)
		printf("MAX ");
	if ((data[0] >> 2) & 0x01)
		printf("CREST ");
	if ((data[0] >> 1) & 0x01)
		printf("REC ");
	if ((data[0] >> 3) & 0x01)
		printf("HOLD ");
	if (data[2] & 0x01)
		printf("DELTA ");
	if (data[9] & 0x01)
		printf("BEEP ");

	//DECODE MAIN DISPLAY
	printf("MAIN: ");

	if ((data[1] >> 7) & 0x01)
		printf("-");

	for (uint8_t i = 0; i < 6; i++)
	{
		//char temp[] = {(char)(48 + decodeDigit(data[2 + i])), '\0'};
		char temp[2];
		temp[0] = 48 + decoder::decodeDigit(data[2 + i]);
		temp[1] = '\0';
		printf(temp);
		if ((data[3 + i] & 0x01) & (i < 4))
			printf(".");
	}
	printf(" ");

	//DECODE UNIT PREFIX FOR MAIN DISPLAY
	if ((data[13] >> 6) & 0x01)
		printf("n");
	if ((data[14] >> 3) & 0x01)
		printf("u");
	if ((data[14] >> 2) & 0x01)
		printf("m");
	if ((data[14] >> 6) & 0x01)
		printf("k");
	if ((data[14] >> 5) & 0x01)
		printf("M");

	//DECODE UNIT FOR MAIN DISPLAY
	if (data[7] & 0x01)
		printf("V ");
	if ((data[13] >> 7) & 0x01)
		printf("A ");
	if ((data[13] >> 5) & 0x01)
		printf("F ");
	if ((data[13] >> 4) & 0x01)
		printf("S ");
	if ((data[14] >> 7) & 0x01)
		printf("D%% ");
	if ((data[14] >> 4) & 0x01)
		printf("Ohm ");
	if ((data[14] >> 1) & 0x01)
		printf("dB ");
	if (data[14] & 0x01)
		printf("Hz ");

	//DC OR AC
	if ((data[0] >> 4) & 0x01)
		printf("DC ");
	if (data[1] & 0x01)
		printf("AC ");

	//DECODE AUXILIARY DISPLAY
	printf("AUX: ");

	if ((data[8] >> 4) & 0x01)
		printf("-");

	for (uint8_t i = 0; i < 4; i++)
	{
		char temp[] = {(char)(48 + decoder::decodeDigit(data[9 + i])), '\0'};
		printf(temp);
		if ((data[10 + i] & 0x01) & (i < 3))
			printf(".");
	}
	printf(" ");

	//DECODE UNIT PREFIX FOR AUXILIARY DISPLAY
	if ((data[8] >> 1) & 0x01)
		printf("m");
	if (data[8] & 0x01)
		printf("u");
	if ((data[13] >> 1) & 0x01)
		printf("k");
	if (data[13] & 0x01)
		printf("M");

	//DECODE UNIT FOR AUXILIARY DISPLAY
	if ((data[13] >> 2) & 0x01)
		printf("Hz");
	if ((data[13] >> 3) & 0x01)
		printf("V");
	if ((data[8] >> 2) & 0x01)
		printf("A");
	if ((data[8] >> 3) & 0x01)
		printf("%%4-20mA");

	printf(" ");
	if ((data[13] >> 2) & 0x01)
		printf("AC");

	//FINISH
	printf("\n");

	//detect low battery
	if ((data[8] >> 7) & 0x01)
		printf(" LOW BAT\n");
}

int8_t decoder::decodeDigit(uint8_t source)
{
	switch (source >> 1)
	{
		case 0b1011111: return 0;
		case 0b1010000: return 1;
		case 0b1101101: return 2;
		case 0b1111100: return 3;
		case 0b1110010: return 4;
		case 0b0111110: return 5;
		case 0b0111111: return 6;
		case 0b1010100: return 7;
		case 0b1111111: return 8;
		case 0b1111110: return 9;
		case 0b1111001: return 'd' - 48;
		case 0b0010000: return 'i' - 48;
		case 0b0111001: return 'o' - 48;
		case 0b0001011: return 'L' - 48;
		case 0b0000000: return ' ' - 48;
		default: return '?' - 48;
	}
}
