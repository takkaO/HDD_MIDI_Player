#ifndef MIDI_HDD_H
#define MIDI_HDD_H

#include "tim.h"
#include "usart.h"

#define PARSE_DATA_BUF_LEN 1024
#define RAW_DATA_BUF_LEN 2048
#define MAX_HDD_NUM 4

typedef struct{
	uint8_t Buffer[RAW_DATA_BUF_LEN];
	uint8_t rxp;
	uint8_t uart_rx_data;
}MIDI_EXTERNAL_GLOBAL_VARIABLES;
extern MIDI_EXTERNAL_GLOBAL_VARIABLES megv;

typedef enum {
	STOP,
	PLAY
}PlayState;

typedef enum {
	FIXED,
	FLEXIBLE
}PlayMethod;

enum PrevData{
	STATUS_BYTE,
	DATA1,
	DATA2,
	NONE
};

enum MIDI_Command{
	NOTE_ON,
	NOTE_OFF
};

typedef struct NOTE{
	uint8_t cmd;
	uint8_t note;
	uint8_t velocity;
}Note;

struct HDD_PARAMS {
	FunctionalState is_enable;
	uint8_t state;
	TIM_HandleTypeDef htim;
	uint8_t note;
};


void MIDI_Variable_Initialize();
void Initialize_HDD(uint8_t hdd_num, TIM_HandleTypeDef htim, FunctionalState is_enable);
FunctionalState Is_HDD_Enable(uint8_t num);
void Enable_HDD(uint8_t hdd_num, FunctionalState enable_disable);
void Change_PlayMethod(PlayMethod pm);
PlayMethod Get_PlayMethod();
void Change_Volume(uint8_t vol);
uint8_t Get_Volume();
void Play_MIDI();
void Parse_MIDI();
void Play_Melody(TIM_HandleTypeDef *htim, enum MIDI_Command cmd, uint8_t note_num);
inline uint8_t is_StatusByte(uint8_t byte) {
	return ((byte & 0x80) ? 1 : 0);
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


#endif
