
#ifndef LCD_H_
#define LCD_H_

// The command code for the LCD.
#define CMD_CLEAR                0x01   // Clear the LCD display.
#define CMD_HOME                 0x02   // Return to home.

// The maskable command code to change the configuration of the LCD.
#define CMD_ENTRY_MODE_SET       0x04   // Bit: 0  0  0  0  0  1  ID S
#define CMD_DISPLAY_CONTROL      0x08   // Bit: 0  0  0  0  1  D  C  B
#define CMD_FUNCTION_SET         0x20   // Bit: 0  0  1  DL N  F  0  0
                              
// The mask to change the configuration of the LCD.
#define MSK_ID                   0x02   // Increment.
#define MSK_S                    0   // Don't shift.
#define MSK_D                    0x04   // On display.
#define MSK_D_OFF                0   // Off display.
#define MSK_C                    0   // Don't display cursor.
#define MSK_B                    0   // Don't blink.
#define MSK_DL_8                 0x10   // Select 8-bit data bus.
#define MSK_DL_4                 0   // Select 4-bit data bus.
#define MSK_N                    0x08   // Select 2 or more-line display.
#define MSK_F                    0   // Select 5 x 8 dots character.

// The maskable command to change the LCD RAM address.
#define CMD_SET_DDRAM_ADDRESS    0x80   // Bit 0 - 7: Address

// The DDRAM address corresponding to 20 * 4 LCD.
#define LCD_LINE_1_ADDR          0
#define LCD_LINE_2_ADDR          0x40
#define LCD_LINE_3_ADDR          0x14
#define LCD_LINE_4_ADDR          0x54

/* port specific elements */

#define lcd_rs  71
//#define lcd_rw  PIN_?
#define lcd_en  73

#define lcd_d0  75
#define lcd_d1  77
#define lcd_d2  79
#define lcd_d3  80

void init_lcd_port();

/* port specific elements end */

/* lcd functions */

void delay_ms( int ms );

void lcd_send_nibble( char n );

void lcd_send_byte( char type, char data );

void lcd_clear();

void lcd_init();


void lcd_gotoxy( char col, char row );

void lcd_putc( char c );

void lcd_puts( char * str, int str_len );

void lcd_putd( int num, int digit_count );


#endif //LCD_H_