/* IO Ports:
0x50 - 0x51 8279
0x60 - 0x6f 8256
0x72        Sound
*/

#define KDC_DATA 0x50
#define KDC_CMD 0x51

#define WRITE_DISPLAY 0x80

void kdc_cmd_out(int data) {
    outp(KDC_CMD, data);
}

void kdc_data_out(int data) {
    outp(KDC_DATA, data);
}

int kdc_cmd_in() {
    return inp(KDC_CMD);
}

int kdc_data_in() {
    return inp(KDC_DATA);
}

void writeLamps(int line, int data) {
    kdc_cmd_out(WRITE_DISPLAY | line);
    kdc_data_out(data);
}

void writeDigits(int digit, int mon, int srv) {
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
    int i;

    init_kdc();

    // lamps
    for (i = 0; i < 8; i++) {
        writeLamps(i,0xFF);
    }

    // 7segment displays
    for (i = 0; i < 8; i++) {
        writeDigits(i,0x03,0x04);
    }

    int last_key = 0;

    // Infinite loop to keep lamps on and scan keyboard
    while (1) {
        int status = kdc_cmd_in();
        if ((status & 0x04) == 0) { // FIFO not empty
            last_key = kdc_data_in();
        }
        writeDigits(0, last_key >> 4, 0);
        writeDigits(1, 0, last_key & 0x0F);
    }
}