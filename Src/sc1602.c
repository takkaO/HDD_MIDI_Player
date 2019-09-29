/*
 * sc1602.c
 *
 *  Created on: 2019/06/08
 *      Author: takkaO
 */

#include "sc1602.h"

/* LCD global variables */
/* Global variable structure */
typedef struct {
	uint8_t current_line;	// LCD current focus line
	enum LCD_MODE mode;		// LCD operating mode
	uint16_t txp;			// LCD transmit data index point
	uint16_t bfp;			// LCD buffered data index point
	struct LCD_LOOP_STRUCT loop_array[LCD_LOOP_ARRAY_NUM]; // LCD command buffer
} LCD_GLOBAL_VARIABLES;
LCD_GLOBAL_VARIABLES lgv;


#ifdef USE_LCD_PRINTF
/***************************************************
 * @fn   : LCD_printf
 * @brief: printf for LCD
 * @param(line): LCD line number
 * @param(*fmt): printf compliant format
 * @sa   : LCD_puts()
 ***************************************************/
void LCD_printf(uint8_t line, const char *fmt, ...) {
	uint8_t buf[64] = { 0 };
	uint8_t line_buf[LINE_LENGTH + 1] = { 0 };
	uint8_t i;
	uint16_t len;
	va_list ap;

	va_start(ap, fmt);
	len = vsnprintf((char *) buf, 64, fmt, ap);

	LCD_Set_Position(1, line);
	for (i = 0; i < LINE_LENGTH; i++) {
		line_buf[i] = buf[i];
	}
	LCD_puts((char *) line_buf);
	if (len > LINE_LENGTH) {
		LCD_putc('\n');
		for (i = 0; i < LINE_LENGTH; i++) {
			line_buf[i] = buf[i + LINE_LENGTH];
		}
		LCD_puts((char *) line_buf);
	}

}
#endif

/***************************************************
 * @fn   : LCD_Loop
 * @brief: LCD operation for async mode
 * @sa   : LCD_Change_Mode()
 ***************************************************/
void LCD_Loop() {
	static enum LCD_LOOP_STATUS lcd_loop_status = CMD_WAIT;
	static uint32_t start_time;

	switch (lcd_loop_status) {
	case CMD_SET_AND_ENABLE:
		LCD_Command_Set(lgv.loop_array[lgv.txp].lcd);
		// Set enable pin HIGH
		HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);

		start_time = HAL_GetTick();
		lcd_loop_status = CMD_BUSY;
		break;
	case CMD_BUSY:
		if (HAL_GetTick() - start_time >= lgv.loop_array[lgv.txp].busy_ms) {
			lgv.txp = AddCounter(lgv.txp, LCD_LOOP_ARRAY_NUM);
			lcd_loop_status = CMD_WAIT;
		}
		break;
	case CMD_WAIT:
		// Set enable pin LOW
		HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
		if (lgv.txp != lgv.bfp) {
			lcd_loop_status = CMD_SET_AND_ENABLE;
		}
		break;
	default:
		break;
	}
}

/***************************************************
 * @fn   : LCD_Initialize
 * @brief: initialize LCD module
 * @sa   : LCD_Variable_Initialize()
 ***************************************************/
void LCD_Initialize() {
	union LCD_TRANSMIT_DATA lcd;
	uint8_t cmd;

	HAL_Delay(45);

	// 8bit mode initialize
	lcd.BYTE = 0;
	lcd.BIT.D4 = 1;
	lcd.BIT.D5 = 1;
	lcd.BIT.D6 = 0;
	lcd.BIT.D7 = 0;
	LCD_Command_Set(lcd);
	LCD_Command_Enable();
	HAL_Delay(5);
	LCD_Command_Set(lcd);
	LCD_Command_Enable();
	HAL_Delay(5);
	LCD_Command_Set(lcd);
	LCD_Command_Enable();
	HAL_Delay(5);

	// set 4bit mode
	lcd.BIT.D4 = 0;
	lcd.BIT.D5 = 1;
	lcd.BIT.D6 = 0;
	lcd.BIT.D7 = 0;
	LCD_Command_Set(lcd);
	LCD_Command_Enable();

	cmd = Get_CMD_Function_Set(0, 1, 0);
	LCD_Send_Command(0, 0, cmd);

	cmd = Get_CMD_Display_On_Off(0, 0, 0);
	LCD_Send_Command(0, 0, cmd);

	LCD_Clear();

	cmd = Get_CMD_Entry_Mode_Set(1, 0);
	LCD_Send_Command(0, 0, cmd);

	cmd = Get_CMD_Display_On_Off(1, 0, 0);
	LCD_Send_Command(0, 0, cmd);

	// additional
	LCD_Set_Position(1, 1);

	// initialize global variables
	LCD_Variable_Initialize();
}

/***************************************************
 * @fn   : LCD_Variable_Initialize
 * @brief: Initialize LCD global variables
 ***************************************************/
void LCD_Variable_Initialize() {
	lgv.current_line = 1;
	lgv.mode = SYNC;
	lgv.txp = 0;
	lgv.bfp = 0;
}

/***************************************************
 * @fn   : LCD_Change_Mode
 * @brief: Change LCD operation mode
 * @param(mode): Next operation mode
 ***************************************************/
void LCD_Change_Mode(enum LCD_MODE mode) {
	lgv.mode = mode;
	if (lgv.mode == SYNC) {
		// Do remain order
		while(lgv.txp != lgv.bfp){
			LCD_Loop();
		}
	}
}

/***************************************************
 * @fn   : LCD_Line_Clear
 * @brief: Clear LCD line character
 * @param(line): LCD line number
 ***************************************************/
void LCD_Line_Clear(uint8_t line){
	if (line > MAX_LINE) {
		return;
	}

	char space[LINE_LENGTH + 1] = { 0 };
	memset(space, ' ', LINE_LENGTH);
	LCD_Set_Position(1, line);
	LCD_puts(space);
}

/***************************************************
 * @fn   : LCD_Clear
 * @brief: Clear LCD all characters
 ***************************************************/
void LCD_Clear() {
	LCD_Send_Command(0, 0, CMD_CLEAR_DISPLAY);
	if (lgv.mode == SYNC) {
		HAL_Delay(2);
	}
}

/***************************************************
 * @fn   : LCD_puts
 * @brief: print characters
 * @param(*str): print string array
 ***************************************************/
void LCD_puts(char *str) {
	while (*str != '\0') {
		LCD_putc(*str);
		str++;
	}
}

/***************************************************
 * @fn   : LCD_putc
 * @brief: print character
 * @param(ch): print character
 ***************************************************/
void LCD_putc(char ch) {
	if (ch == '\r') {
		LCD_Set_Position(1, lgv.current_line);
	}
	else if (ch == '\n') {
		lgv.current_line = lgv.current_line == MAX_LINE ? 1 : lgv.current_line + 1;
		LCD_Set_Position(1, lgv.current_line);
	}
	else {
		LCD_Send_Command(1, 0, ch);
	}
}

/***************************************************
 * @fn   : LCD_Set_Position
 * @brief: LCD set cursor position
 * @param(x): x axis new position
 * @param(y): y axis new position
 ***************************************************/
void LCD_Set_Position(uint8_t x, uint8_t y) {
	lgv.current_line = y;
	switch (y) {
	case 1:
		LCD_Send_Command(0, 0, 0x80 + x - 1);
		break;
	case 2:
		LCD_Send_Command(0, 0, 0xC0 + x - 1);
		break;
	default:
		LCD_Send_Command(0, 0, 0x80);
	}
}

/***************************************************
 * @fn   : LCD_Send_Command
 * @brief: Send command to LCD
 * @param(rs) : Register select (0/1)
 * @param(rw) : Read/Write (0/1)
 * @param(cmd): Command
 ***************************************************/
void LCD_Send_Command(uint8_t rs, uint8_t rw, uint8_t cmd) {
	union LCD_TRANSMIT_DATA lcd;
	lcd.BYTE = 0;
	lcd.BIT.RS = rs;
	lcd.BIT.RW = rw;

	// send MSB
	lcd.BYTE &= 0xF0;
	lcd.BYTE |= (cmd >> 4);
	if (lgv.mode == ASYNC) {
		lgv.loop_array[lgv.bfp].lcd = lcd;
		lgv.loop_array[lgv.bfp].busy_ms = 2;
		lgv.bfp = AddCounter(lgv.bfp, LCD_LOOP_ARRAY_NUM);
	}
	else if (lgv.mode == SYNC) {
		LCD_Command_Set(lcd);
		LCD_Command_Enable();
	}

	// send LSB
	lcd.BYTE &= 0xF0;
	lcd.BYTE |= (cmd & 0x0F);
	if (lgv.mode == ASYNC) {
		lgv.loop_array[lgv.bfp].lcd = lcd;
		if (cmd == CMD_CLEAR_DISPLAY) {
			// Long wait time for display clear
			lgv.loop_array[lgv.bfp].busy_ms = 5;
		}
		else {
			lgv.loop_array[lgv.bfp].busy_ms = 2;
		}
		lgv.bfp = AddCounter(lgv.bfp, LCD_LOOP_ARRAY_NUM);
	}
	else if (lgv.mode == SYNC) {
		LCD_Command_Set(lcd);
		LCD_Command_Enable();
	}

}

/***************************************************
 * @fn   : LCD_Command_Set
 * @brief: Set GPIO H/L according to command
 * @param(lcd): command
 ***************************************************/
void LCD_Command_Set(union LCD_TRANSMIT_DATA lcd) {
	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, lcd.BIT.RS);
	HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, lcd.BIT.RW);

	HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, lcd.BIT.D7);
	HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, lcd.BIT.D6);
	HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, lcd.BIT.D5);
	HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, lcd.BIT.D4);
}

/***************************************************
 * @fn   : LCD_Command_Enable
 * @brief: LCD command enable
 ***************************************************/
void LCD_Command_Enable() {
	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
	DELAY_US(37);
	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
}

/***************************************************
 * @fn   : Get_CMD_Function_Set
 * @brief: Make function_set command
 * @param(DL): Interface data (8bit/4bit)
 * @param(N) : Number of line (2/1)
 * @param(F) : Font size
 * @return   : Made command byte
 ***************************************************/
uint8_t Get_CMD_Function_Set(uint8_t DL, uint8_t N, uint8_t F) {
	/*
	 * DL: (1) 8bit mode, (0) 4bit mode
	 * N : (1) 2 line, (0) 1 line
	 * F : Font size (1) 5x11, (0)5x8
	 */
	union LCD_REGISTER_FIELD lrf;
	lrf.BYTE = CMD_FUNCTION_SET;
	lrf.BIT.D4 = DL;
	lrf.BIT.D3 = N;
	lrf.BIT.D2 = F;
	return lrf.BYTE;
}

/***************************************************
 * @fn   : Get_CMD_Cursor_Display_Shift
 * @brief: Make cursor_or_display_shift command
 * @param(SC): Display cursor
 * @param(RL): Right/Left
 * @return   : Made command byte
 ***************************************************/
uint8_t Get_CMD_Cursor_Display_Shift(uint8_t SC, uint8_t RL) {
	union LCD_REGISTER_FIELD lrf;
	lrf.BYTE = CMD_CURSOR_OR_DISPLAY_SHIFT;
	lrf.BIT.D3 = SC;
	lrf.BIT.D2 = RL;
	return lrf.BYTE;
}

/***************************************************
 * @fn   : Get_CMD_Display_On_Off
 * @brief: Make display_on/off command
 * @param(D): Display ON/OFF
 * @param(C): Cursor ON/OFF
 * @param(B): Blink cursor ON/OFF
 * @return   : Made command byte
 ***************************************************/
uint8_t Get_CMD_Display_On_Off(uint8_t D, uint8_t C, uint8_t B) {
	/*
	 * D : (1) display on, (0) display off
	 * C : (1) cursor on, (0) cursor off
	 * B : (1) blink on, (0) blink off
	 */
	union LCD_REGISTER_FIELD lrf;
	lrf.BYTE = CMD_DISPLAY_ON_OFF;
	lrf.BIT.D2 = D;
	lrf.BIT.D1 = C;
	lrf.BIT.D0 = B;
	return lrf.BYTE;
}

/***************************************************
 * @fn   : Get_CMD_Entry_Mode_Set
 * @brief: Make entry_mode_set command
 * @param(ID): Increment/decrement
 * @param(S) : Display shift
 * @return   : Made command byte
 ***************************************************/
uint8_t Get_CMD_Entry_Mode_Set(uint8_t ID, uint8_t S) {
	union LCD_REGISTER_FIELD lrf;
	lrf.BYTE = CMD_ENTRY_MODE_SET;
	lrf.BIT.D1 = ID;
	lrf.BIT.D0 = S;
	return lrf.BYTE;
}

