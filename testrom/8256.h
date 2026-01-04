/**
 * @file 8256.h
 * @brief Header file for Intel 8256 I8256 controller interface
 *
 * @author stonedDiscord
 * @date 15.12.2025
 */

#include <stdbool.h>

#define I8256_CMD1      0x00
#define I8256_CMD2      0x01
#define I8256_CMD3      0x02
#define I8256_MODE      0x03
#define I8256_PORT1C    0x04
#define I8256_INTEN     0x05
#define I8256_INTAD     0x06
#define I8256_BUFFER    0x07
#define I8256_PORT1     0x08
#define I8256_PORT2     0x09
#define I8256_TIMER1    0x0a
#define I8256_TIMER2    0x0b
#define I8256_TIMER3    0x0c
#define I8256_TIMER4    0x0d
#define I8256_TIMER5    0x0e
#define I8256_STATUS    0x0f


#define I8256_CMD1_FRQ_16    0x00
#define I8256_CMD1_FRQ_1K    0x01
#define I8256_CMD1_8085      0x00
#define I8256_CMD1_8086      0x02
#define I8256_CMD1_BITI      0x04
#define I8256_CMD1_BRKI      0x08
#define I8256_CMD1_STOP_1    0x00
#define I8256_CMD1_STOP_15   0x10
#define I8256_CMD1_STOP_2    0x20
#define I8256_CMD1_STOP_075  0x30
#define I8256_CMD1_CHARLEN_8 0x00
#define I8256_CMD1_CHARLEN_7 0x40
#define I8256_CMD1_CHARLEN_6 0x80
#define I8256_CMD1_CHARLEN_5 0xC0

#define I8256_CMD2_SCLK_DIV5 0x00
#define I8256_CMD2_SCLK_DIV3 0x10
#define I8256_CMD2_SCLK_DIV2 0x20
#define I8256_CMD2_SCLK_DIV1 0x30
#define I8256_CMD2_ODD_PARITY  0x00
#define I8256_CMD2_EVEN_PARITY 0x40
#define I8256_CMD2_NO_PARITY   0x00
#define I8256_CMD2_PARITY      0x80

#define I8256_CMD3_RESET    0x01
#define I8256_CMD3_TBRK     0x02
#define I8256_CMD3_SBRK     0x04
#define I8256_CMD3_END      0x08
#define I8256_CMD3_NIE      0x10
#define I8256_CMD3_IAE      0x20
#define I8256_CMD3_RXE      0x40
#define I8256_CMD3_SET      0x80

#define I8256_MODE_PORT2C_II   0x00
#define I8256_MODE_PORT2C_IO   0x01
#define I8256_MODE_PORT2C_OI   0x02
#define I8256_MODE_PORT2C_OO   0x03
#define I8256_MODE_PORT2C_HI   0x04
#define I8256_MODE_PORT2C_HO   0x05
#define I8256_MODE_PORT2C_DNU  0x06
#define I8256_MODE_PORT2C_TEST 0x07
#define I8256_MODE_CT2_TMR  0x00
#define I8256_MODE_CT2_CNT  0x08
#define I8256_MODE_CT3_TMR  0x00
#define I8256_MODE_CT3_CNT  0x10
#define I8256_MODE_T5C      0x20
#define I8256_MODE_T24      0x40
#define I8256_MODE_T35      0x80

#define I8256_STATUS_FE    0x01
#define I8256_STATUS_OE    0x02
#define I8256_STATUS_PE    0x04
#define I8256_STATUS_BD    0x08
#define I8256_STATUS_TRE   0x10
#define I8256_STATUS_TBE   0x20
#define I8256_STATUS_RBF   0x40
#define I8256_STATUS_INT   0x80
