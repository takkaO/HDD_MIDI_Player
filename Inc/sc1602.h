#ifndef SC1602_H
#define SC1602_H

#include "gpio.h"
#include <string.h>

#define CMD_FUNCTION_SET 			0b00100000
#define CMD_CURSOR_OR_DISPLAY_SHIFT 0b00010000
#define CMD_DISPLAY_ON_OFF			0b00001000
#define CMD_ENTRY_MODE_SET			0b00000100
#define CMD_RETURN_HOME				0b00000010
#define CMD_CLEAR_DISPLAY			0b00000001

#define MAX_LINE 2		// LCD max line
#define LINE_LENGTH 16	// LCD line length
#define LCD_LOOP_ARRAY_NUM 1024

// Use printf function
#define USE_LCD_PRINTF

union LCD_TRANSMIT_DATA {
	uint8_t BYTE;
	struct {
		uint8_t D4 :1;	//LSB
		uint8_t D5 :1;
		uint8_t D6 :1;
		uint8_t D7 :1;
		uint8_t RS :1;
		uint8_t RW :1;
		uint8_t NONE :2; //MSB
	} BIT;
};

union LCD_REGISTER_FIELD {
	uint8_t BYTE;
	struct {
		uint8_t D0 :1;
		uint8_t D1 :1;
		uint8_t D2 :1;
		uint8_t D3 :1;
		uint8_t D4 :1;
		uint8_t D5 :1;
		uint8_t D6 :1;
		uint8_t D7 :1;
	} BIT;
};

struct LCD_LOOP_STRUCT {
	union LCD_TRANSMIT_DATA lcd;
	uint8_t busy_ms;
};

enum LCD_LOOP_STATUS {
	CMD_SET_AND_ENABLE,
	CMD_BUSY,
	CMD_WAIT
};

enum LCD_MODE {
	SYNC,
	ASYNC
};


#define CLOCK_TIME (1000000000.0 / 168000000) //(ns)
inline void DELAY_US(uint16_t us) {
	uint32_t c = (1000 / CLOCK_TIME) * us;
	while (c) {
		asm("NOP");
		c--;
	}
}

#ifndef ADD_COUNTER_FUNCTION
#define ADD_COUNTER_FUNCTION
inline uint16_t AddCounter(uint16_t now_value, uint16_t limit) {
	now_value++;
	if (now_value == limit) {
		now_value = 0;
	}
	return now_value;
}
#endif

#ifdef USE_LCD_PRINTF
	#include <stdarg.h>
	#include <stdio.h>
	void LCD_printf(uint8_t line, const char *fmt, ...);
#endif
void LCD_Loop();
void LCD_Initialize();
void LCD_Variable_Initialize();
void LCD_Change_Mode(enum LCD_MODE mode);
void LCD_Line_Clear(uint8_t line);
void LCD_Clear();
void LCD_puts(char *str);
void LCD_putc(char ch);
void LCD_Set_Position(uint8_t x, uint8_t y);
void LCD_Send_Command(uint8_t rs, uint8_t rw, uint8_t cmd);
void LCD_Command_Set(union LCD_TRANSMIT_DATA lcd);
void LCD_Command_Enable();
uint8_t Get_CMD_Function_Set(uint8_t DL, uint8_t N, uint8_t F);
uint8_t Get_CMD_Cursor_Display_Shift(uint8_t SC, uint8_t RL);
uint8_t Get_CMD_Display_On_Off(uint8_t D, uint8_t C, uint8_t B);
uint8_t Get_CMD_Entry_Mode_Set(uint8_t ID, uint8_t S);

#endif
