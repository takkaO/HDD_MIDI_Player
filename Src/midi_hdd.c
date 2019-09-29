#include "midi_hdd.h"

uint16_t psc[128] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 71, 73, 60, 77, 85, 52, 54, 44, 56, 71,
		57, 35, 84, 46, 66, 37, 33, 28, 26, 34, 30, 22, 22, 25, 30, 17, 16, 18, 15, 17, 30, 18, 34,
		9, 10, 8, 46, 8, 14, 8, 42, 72, 5, 6, 36, 42, 5, 5, 46, 3, 6, 6, 42, 3, 2, 6, 3, 3, 2, 2, 2,
		1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

uint16_t arr[128] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62499, 56898, 64273, 48582, 39490, 62238,
		56916, 63999, 48582, 35460, 42512, 64791, 25980, 43768, 29274, 48582, 51270, 56170, 57970,
		41896, 44664, 56916, 53972, 44664, 35731, 57970, 57688, 48582, 54877, 45976, 24973, 38707,
		19812, 65394, 55943, 64791, 11693, 57553, 32652, 51281, 10147, 5635, 64864, 52477, 9370,
		7610, 51501, 48582, 5846, 64981, 35025, 33072, 5073, 51575, 64864, 26238, 43362, 40908,
		51501, 48582, 45888, 64981, 61328, 57877, 54627, 51575, 48641, 45917, 28880, 40908, 25750,
		36436, 34383, 64922, 61276, 57877, 54586, 51538, 48648, 45917, 43346, 40908, 38605, 36436,
		34399, 32475, 30650, 28926, 27303, 25769, 24323, 22958, 21673, 20454, 19307, 18222, 17199,
		16233, 15321, 14463, 13651, 12884, 12161, 11479, 10834, 10226, 9652, 9111, 8599, 8116, 7661,
		7231, 6825, 6442, 6080, 5739 };


typedef struct{
	Note v[PARSE_DATA_BUF_LEN];
	uint8_t txp;
	uint16_t cbufp;
	PlayMethod play_method;
	struct HDD_PARAMS hdd[MAX_HDD_NUM];
	uint8_t volume;
}MIDI_GLOBAL_VARIABLES;

MIDI_GLOBAL_VARIABLES mgv;
MIDI_EXTERNAL_GLOBAL_VARIABLES megv;


void MIDI_Variable_Initialize(){
	megv.rxp = 0;
	megv.uart_rx_data = 0;

	mgv.cbufp = 0;
	mgv.txp = 0;
	mgv.play_method = FIXED;
	mgv.volume = 50;
}


void Initialize_HDD(uint8_t hdd_num, TIM_HandleTypeDef htim, FunctionalState is_enable) {
	if (MAX_HDD_NUM < hdd_num) {
		return;
	}
	mgv.hdd[hdd_num].is_enable = is_enable;
	mgv.hdd[hdd_num].state = STOP;
	mgv.hdd[hdd_num].htim = htim;
	mgv.hdd[hdd_num].note = 0xFF;
}

FunctionalState Is_HDD_Enable(uint8_t num) {
	if (num >= MAX_HDD_NUM) {
		return DISABLE;
	}
	return mgv.hdd[num].is_enable;
}

void Enable_HDD(uint8_t hdd_num, FunctionalState enable_disable){
	if (MAX_HDD_NUM < hdd_num) {
		return;
	}
	mgv.hdd[hdd_num].state = STOP;
	Play_Melody(&mgv.hdd[hdd_num].htim, NOTE_OFF, mgv.hdd[hdd_num].note);
	mgv.hdd[hdd_num].is_enable = enable_disable;
}

void Change_PlayMethod(PlayMethod pm){
	for(uint8_t i=0; i<MAX_HDD_NUM; i++){
		// Reset HDD state
		mgv.hdd[i].state = STOP;
		Play_Melody(&mgv.hdd[i].htim, NOTE_OFF, mgv.hdd[i].note);
	}
	mgv.play_method = pm;
}

PlayMethod Get_PlayMethod() {
	return mgv.play_method;
}

void Change_Volume(uint8_t vol) {
	if (100 < vol) {
		return;
	}
	mgv.volume = vol;
}

uint8_t Get_Volume(){
	return mgv.volume;
}

void Play_MIDI() {
	int i;
	static uint16_t pbufp = 0;
	if (mgv.cbufp == pbufp) {
		return;
	}

	pbufp = AddCounter(pbufp, PARSE_DATA_BUF_LEN);

	if (mgv.play_method == FLEXIBLE) {
		if ((mgv.v[pbufp].cmd & 0xF0) == 0x80) {
			// note off
			for (i = 0; i < MAX_HDD_NUM; i++) {
				if (mgv.hdd[i].is_enable == DISABLE) {
					continue;
				}
				if (mgv.hdd[i].state == PLAY && mgv.hdd[i].note == mgv.v[pbufp].note) {
					mgv.hdd[i].state = STOP;
					Play_Melody(&mgv.hdd[i].htim, NOTE_OFF, mgv.v[pbufp].note);
					break;
				}
			}
		}
		else if ((mgv.v[pbufp].cmd & 0xF0) == 0x90) {
			// note on
			for (i = 0; i < MAX_HDD_NUM; i++) {
				if (mgv.hdd[i].is_enable == DISABLE) {
					continue;
				}
				if (mgv.hdd[i].state == STOP) {
					mgv.hdd[i].state = PLAY;
					if (mgv.hdd[i].note != mgv.v[pbufp].note) {
						mgv.hdd[i].note = mgv.v[pbufp].note;
					}
					else {
						__HAL_TIM_SET_COMPARE(&mgv.hdd[i].htim, TIM_CHANNEL_1, 0);
						HAL_Delay(1);
					}
					Play_Melody(&mgv.hdd[i].htim, NOTE_ON, mgv.v[pbufp].note);
					break;
				}
			}
		}
		return;
	}
	else if (mgv.play_method == FIXED) {
		switch (mgv.v[pbufp].cmd) {
		case 0x80:
			// note off
			if (mgv.hdd[0].is_enable && mgv.hdd[0].state == PLAY) {
				if (mgv.hdd[0].note == mgv.v[pbufp].note) {
					mgv.hdd[0].state = STOP;
					Play_Melody(&mgv.hdd[0].htim, NOTE_OFF, mgv.v[pbufp].note);
				}
			}
			break;
		case 0x90:
			// note on
			if (mgv.hdd[0].is_enable && mgv.hdd[0].state == STOP) {
				mgv.hdd[0].state = PLAY;
				if (mgv.hdd[0].note != mgv.v[pbufp].note) {
					mgv.hdd[0].note = mgv.v[pbufp].note;
				}
				else {
					__HAL_TIM_SET_COMPARE(&mgv.hdd[0].htim, TIM_CHANNEL_1, 0);
					HAL_Delay(1);
				}
				Play_Melody(&mgv.hdd[0].htim, NOTE_ON, mgv.v[pbufp].note);
			}
			break;
		case 0x81:
			// note off
			if (mgv.hdd[1].is_enable && mgv.hdd[1].state == PLAY) {
				if (mgv.hdd[1].note == mgv.v[pbufp].note) {
					mgv.hdd[1].state = STOP;
					Play_Melody(&mgv.hdd[1].htim, NOTE_OFF, mgv.v[pbufp].note);
				}
			}
			break;
		case 0x91:
			// note on
			if (mgv.hdd[1].is_enable && mgv.hdd[1].state == STOP) {
				mgv.hdd[1].state = PLAY;
				if (mgv.hdd[1].note != mgv.v[pbufp].note) {
					mgv.hdd[1].note = mgv.v[pbufp].note;
				}
				else {
					__HAL_TIM_SET_COMPARE(&mgv.hdd[1].htim, TIM_CHANNEL_1, 0);
					HAL_Delay(1);
				}
				Play_Melody(&mgv.hdd[1].htim, NOTE_ON, mgv.v[pbufp].note);
			}
			break;
		case 0x82:
			// note off
			if (mgv.hdd[2].is_enable && mgv.hdd[2].state == PLAY) {
				if (mgv.hdd[2].note == mgv.v[pbufp].note) {
					mgv.hdd[2].state = STOP;
					Play_Melody(&mgv.hdd[2].htim, NOTE_OFF, mgv.v[pbufp].note);
				}
			}
			break;
		case 0x92:
			// note on
			if (mgv.hdd[2].is_enable && mgv.hdd[2].state == STOP) {
				mgv.hdd[2].state = PLAY;
				if (mgv.hdd[2].note != mgv.v[pbufp].note) {
					mgv.hdd[2].note = mgv.v[pbufp].note;
				}
				else {
					__HAL_TIM_SET_COMPARE(&mgv.hdd[2].htim, TIM_CHANNEL_1, 0);
					HAL_Delay(1);
				}
				Play_Melody(&mgv.hdd[2].htim, NOTE_ON, mgv.v[pbufp].note);
			}
			break;
		case 0x83:
			// note off
			if (mgv.hdd[3].is_enable && mgv.hdd[3].state == PLAY) {
				if (mgv.hdd[3].note == mgv.v[pbufp].note) {
					mgv.hdd[3].state = STOP;
					Play_Melody(&mgv.hdd[3].htim, NOTE_OFF, mgv.v[pbufp].note);
				}
			}
			break;
		case 0x93:
			// note on
			if (mgv.hdd[3].is_enable && mgv.hdd[3].state == STOP) {
				mgv.hdd[3].state = PLAY;
				if (mgv.hdd[3].note != mgv.v[pbufp].note) {
					mgv.hdd[3].note = mgv.v[pbufp].note;
				}
				else {
					__HAL_TIM_SET_COMPARE(&mgv.hdd[3].htim, TIM_CHANNEL_1, 0);
					HAL_Delay(1);
				}
				Play_Melody(&mgv.hdd[3].htim, NOTE_ON, mgv.v[pbufp].note);
			}
			break;
		default:
			return;
		}
	}
}

void Parse_MIDI() {
	static enum PrevData pd = NONE;
	static uint16_t wbufp = 0;
	static uint8_t status;

	if (mgv.txp == megv.rxp) {
		return;
	}

	if (is_StatusByte(megv.Buffer[mgv.txp])) {
		// TODO:破棄処理
		wbufp = AddCounter(wbufp, PARSE_DATA_BUF_LEN);
		mgv.v[wbufp].cmd = megv.Buffer[mgv.txp];
		status = mgv.v[wbufp].cmd;
		pd = STATUS_BYTE;
	}
	else if (pd == DATA2 || pd == STATUS_BYTE) {
		if (pd == DATA2) {
			wbufp = AddCounter(wbufp, PARSE_DATA_BUF_LEN);
			mgv.v[wbufp].cmd = status;
		}
		mgv.v[wbufp].note = megv.Buffer[mgv.txp];
		pd = DATA1;
	}
	else if (pd == DATA1) {
		mgv.v[wbufp].velocity = megv.Buffer[mgv.txp];
		pd = DATA2;
		mgv.cbufp = wbufp;
		//print(&huart3, "%02x %02d\r\n", v[wbufp].cmd, v[wbufp].note);
	}
	mgv.txp = AddCounter(mgv.txp, 2048);
}

void Change_TIM_Freq(TIM_HandleTypeDef *htim, uint8_t note_num) {
	__HAL_TIM_SET_PRESCALER(htim, psc[note_num]);
	__HAL_TIM_SET_AUTORELOAD(htim, arr[note_num]);
}

void Play_Melody(TIM_HandleTypeDef *htim, enum MIDI_Command cmd, uint8_t note_num) {
	if (note_num < 12 || 127 < note_num) {
		return;
	}

	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0);

	//要検証
	if (cmd == NOTE_OFF) {
		return;
	}
	Change_TIM_Freq(htim, note_num);

	// Re-calculate volume to float (Cut over 90%)
	double _volume = (mgv.volume > 90 ? 90 : mgv.volume) * 0.01;

	uint16_t duty = __HAL_TIM_GET_AUTORELOAD(htim) * _volume;
	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, duty);
}
