/*
 * eeprom.h
 *
 *  Created on: 2019/09/29
 *      Author: takkaO
 */

#ifndef EEPROM_H
#define EEPROM_H

#include "i2c.h"

#define EEPROM_USE_LCD
#ifdef EEPROM_USE_LCD
#include "sc1602.h"
#endif

#define SLAVE_ADDRESS               0b10100000
#define EEPROM_DATA_STRUCTURE_LEN   8
#define EEPROM_DATA_LEN             6
#define HDD_POINTER                 4  // hdd setting pointer

typedef union {
	uint8_t data[EEPROM_DATA_LEN];
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

void EEPROM_Initialize(I2C_HandleTypeDef *_hi2c);
void Save_Settings(DATA_STRUCTURE ds, uint16_t size);
DATA_STRUCTURE Load_Settings(uint16_t addr, uint16_t size);

#endif
