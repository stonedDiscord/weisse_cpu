/* IO Ports:
0x50 - 0x51 8279
0x60 - 0x6f 8256
0x72        Sound

*/

#include <stdint.h>
#include "8279.h"

#define KDC_DATA 0x50
#define KDC_CMD 0x51

void kdc_cmd_out(uint8_t data) {
    uint8_t test = data;
	__asm
        OUT KDC_CMD
    __endasm;
}

void kdc_data_out(uint8_t data) {
    uint8_t test = data;
	__asm
        OUT KDC_DATA
    __endasm;
}

uint8_t kdc_cmd_in() {
    uint8_t out;
	__asm
        POP HL
        IN KDC_CMD
        MOV L,A
        PUSH HL
    __endasm;
    return out;
}

uint8_t kdc_data_in() {
    uint8_t out;
	__asm
        POP HL
        IN KDC_DATA
        MOV L,A
        PUSH HL
    __endasm;
    return out;
}

uint8_t readSram(uint8_t addr) {
    kdc_cmd_out(I8279_READ_SENSOR_RAM | (addr & 7));
    return kdc_data_in();
}

void writeLamps(uint8_t line, uint8_t data) {
    kdc_cmd_out(I8279_WRITE_DISPLAY_RAM | (line & 7));
    kdc_data_out(data);
}

void writeDigits(uint8_t digit, uint8_t mon, uint8_t srv) {
    kdc_cmd_out(I8279_WRITE_DISPLAY_RAM | ((digit & 7) + 8));
    kdc_data_out((mon << 4) | srv);
}

void init_kdc() {
    kdc_cmd_out(I8279_MODE_SET |
                I8279_MODE_DISPLAY_16BIT |
                I8279_MODE_DISPLAY_LE |
                I8279_MODE_KEYBOARD_ENCODED |
                I8279_MODE_KEYBOARD_SENSOR_MATRIX);
    kdc_cmd_out(I8279_CLOCK_DIVIDER_SET | 30);
    kdc_cmd_out(I8279_CLEAR | I8279_CLEAR_ALL);
    kdc_cmd_out(I8279_END_INTERRUPT);
}

void delay(uint16_t ms) {
    uint16_t i;
    for (i = 0; i < ms; i++) {
        ;
    }
}

void main(void) {
    uint8_t i;

    init_kdc();

    // lamps
    for (i = 0; i < 8; i++) {
        writeLamps(i,0xFF);
    }

    // 7segment displays
    for (i = 0; i < 8; i++) {
        writeDigits(i,0x03,0x04);
    }

    uint8_t last_key = 0;

    // Infinite loop to keep lamps on and scan keyboard
    while (1) {
        delay(150);
        last_key = readSram(i);
        delay(150);
        writeDigits(i, last_key >> 4, last_key & 0x0F);
        delay(150);
        writeLamps(i, last_key);
        delay(150);
     
        kdc_cmd_out(I8279_END_INTERRUPT);

        i++;
        if (i >= 8) {
            i = 0;
        }

    }
}
