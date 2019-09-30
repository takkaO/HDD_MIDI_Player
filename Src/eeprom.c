/*
 * eeprom.c
 *
 *  Created on: 2019/09/29
 *      Author: takkaO
 */

#include "eeprom.h"

I2C_HandleTypeDef *hi2c;

void EEPROM_Initialize(I2C_HandleTypeDef *_hi2c){
	hi2c = _hi2c;
}



void Save_Settings(DATA_STRUCTURE ds, uint16_t size){
#ifdef EEPROM_USE_LCD
	// Need initialize before.
	LCD_Clear();
	LCD_Set_Position(1, 2);
	LCD_puts("Wait EEPROM...");
#endif
	while(HAL_I2C_IsDeviceReady(hi2c, SLAVE_ADDRESS, 1, 10) != HAL_OK);

	HAL_I2C_Master_Transmit(hi2c, SLAVE_ADDRESS, (uint8_t *) &ds.data, size, 100);

#ifdef EEPROM_USE_LCD
	LCD_Clear();
	LCD_Set_Position(1, 1);
	LCD_puts("Write done!");
	LCD_Set_Position(1, 2);
	LCD_puts("Wait EEPROM...");
#endif
	while(HAL_I2C_IsDeviceReady(hi2c, SLAVE_ADDRESS, 1, 100) != HAL_OK);

#ifdef EEPROM_USE_LCD
	LCD_Clear();
	LCD_Set_Position(1, 1);
	LCD_puts("Save complete!");
#endif
}


DATA_STRUCTURE Load_Settings(uint16_t addr, uint16_t size){
	DATA_STRUCTURE ds;
	ds.BYTE.addr_high = (addr >> 8);
	ds.BYTE.addr_low = (addr >> 0);

#ifdef EEPROM_USE_LCD
	// Need initialize before.
	LCD_Clear();
	LCD_Set_Position(1, 2);
	LCD_puts("Wait EEPROM...");
#endif
	while(HAL_I2C_IsDeviceReady(hi2c, SLAVE_ADDRESS, 1, 10) != HAL_OK);

	// Set read start address
	HAL_I2C_Master_Transmit(hi2c, SLAVE_ADDRESS, (uint8_t *) &ds.data, 2, 100);
#ifdef EEPROM_USE_LCD
	LCD_Clear();
	LCD_Set_Position(1, 1);
	LCD_puts("Transmit addr...");
	LCD_Set_Position(1, 2);
	LCD_puts("Wait EEPROM...");
#endif
	while(HAL_I2C_IsDeviceReady(hi2c, SLAVE_ADDRESS, 1, 10) != HAL_OK);

	// Skip addr pointer
	HAL_I2C_Master_Receive(hi2c, SLAVE_ADDRESS, (uint8_t *) &ds.data[2], size, 100);
#ifdef EEPROM_USE_LCD
	LCD_Clear();
	LCD_Set_Position(1, 1);
	LCD_puts("Write done!");
	LCD_Set_Position(1, 2);
	LCD_puts("Wait EEPROM...");
#endif
	while(HAL_I2C_IsDeviceReady(hi2c, SLAVE_ADDRESS, 1, 10) != HAL_OK);

	return ds;
}
