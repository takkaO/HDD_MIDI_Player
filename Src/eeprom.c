/*
 * eeprom.c
 *
 *  Created on: 2019/09/29
 *      Author: takkaO
 */

#include "eeprom.h"
#include "usart.h"

void Save_Settings(DATA_STRUCTURE ds){
	uint16_t slave_addr = 0b10100000;
	uint8_t d[EEPROM_DATA_LENGTH] = {0, 0, 1, 2, 3, 4, 5, 6};
	HAL_StatusTypeDef ret = HAL_BUSY;
	while(ret != HAL_OK){
		ret = HAL_I2C_IsDeviceReady(&hi2c1, slave_addr, 1, HAL_MAX_DELAY);
	}

	println("f");
	HAL_I2C_Master_Transmit(&hi2c1, slave_addr, &d, EEPROM_DATA_LENGTH, 100);
	println("w");
	while(HAL_I2C_IsDeviceReady(&hi2c1, slave_addr, 5, 100) != HAL_OK);
	println("c");
}

DATA_STRUCTURE Load_Settings(uint16_t addr){
	DATA_STRUCTURE ds;
	ds.BYTE.addr_high = (addr >> 8);
	ds.BYTE.addr_low = (addr >> 0);

	// Set read start address
	HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS, (uint8_t *) &ds.data, 2, 100);
	//while(HAL_I2C_IsDeviceReady(&hi2c1, SLAVE_ADDRESS, 5, 100) != HAL_OK);

	HAL_I2C_Master_Receive(&hi2c1, SLAVE_ADDRESS, (uint8_t *) &ds.data, 5, 100);
	return ds;
}
