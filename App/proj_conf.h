/* proj_conf.h
 *
 * Created on: Mar 7, 2022
 * Author: Martin Danek, martin@embedblog.eu
 */

#ifndef PROJ_CONF_H_
#define PROJ_CONF_H_

#include <stdint.h>

constexpr uint8_t EEPROM_VALID_FLAG = 0xC4;
constexpr uint8_t EEPROM_VALID_ADDR = 0x00;

constexpr uint8_t EEPROM_CONF_ADDR = 0x01;

constexpr uint_fast16_t CLOCK_DELAY_US = 100;

enum class Mode: uint8_t
{
	SEND_1HZ,
	SEND_5HZ,
	STOP,
	SEND_SINGLE
};


#endif /* PROJ_CONF_H_ */
