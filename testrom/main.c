/* IO Ports:
0x50 - 0x51 8279
0x60 - 0x6f 8256
0x72        Sound

*/

#include <stdint.h>

#define KDC_DATA 0x50
#define KDC_CMD 0x51
#define WRITE_DISPLAY 0x80
#define READ_SRAM 0x40

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
	__asm
        IN KDC_CMD
    __endasm;
}

uint8_t kdc_data_in() {
	__asm
        IN KDC_DATA
    __endasm;
}

uint8_t readSram(uint8_t addr) {
    kdc_cmd_out(READ_SRAM | addr);
    return kdc_data_in();
}

void writeLamps(uint8_t line, uint8_t data) {
    kdc_cmd_out(WRITE_DISPLAY | line);
    kdc_data_out(data);
}

void writeDigits(uint8_t digit, uint8_t mon, uint8_t srv) {
    kdc_cmd_out(WRITE_DISPLAY | digit + 8);
    kdc_data_out((mon << 4) | srv);
}

void init_kdc() {
    kdc_cmd_out(0x0C);
    kdc_cmd_out(0x3E);
    kdc_cmd_out(0xC1);
    kdc_cmd_out(0xE0);
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
        uint8_t last_key = readSram(i);

        writeDigits(i, last_key >> 4, last_key & 0x0F);
        writeLamps(i, last_key);

        i++;
        i = i & 7;
    }
}