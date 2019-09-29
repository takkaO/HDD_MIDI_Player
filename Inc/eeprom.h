/*
 * eeprom.h
 *
 *  Created on: 2019/09/29
 *      Author: takkaO
 */

#ifndef EEPROM_H
#define EEPROM_H

#include "i2c.h"

#define SLAVE_ADDRESS   0b10100000
#define EEPROM_DATA_LENGTH     8

typedef union {
	uint8_t data[EEPROM_DATA_LENGTH];
	struct {
		uint8_t addr_high;
		uint8_t addr_low;
		uint8_t play_method;
		uint8_t volume;
		uint8_t hdd1;
		uint8_t hdd2;
		uint8_t hdd3;
		uint8_t hdd4;
	}BYTE;
}DATA_STRUCTURE;

void Save_Settings(DATA_STRUCTURE ds);
DATA_STRUCTURE Load_Settings(uint16_t addr);

#endif
