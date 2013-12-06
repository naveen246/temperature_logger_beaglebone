
#include <math.h>
#include "lcd.h"
#include "io_lib.h"

void init_lcd_port() {
    init_gpio( lcd_rs );
    init_gpio( lcd_en );
    init_gpio( lcd_d0 );
    init_gpio( lcd_d1 );
    init_gpio( lcd_d2 );
    init_gpio( lcd_d3 );
    
    gpio_direction_out( lcd_rs );
    gpio_direction_out( lcd_en );
    gpio_direction_out( lcd_d0 );
    gpio_direction_out( lcd_d1 );
    gpio_direction_out( lcd_d2 );
    gpio_direction_out( lcd_d3 );
}

/* lcd functions */

void delay_ms( int ms ) {
    if( ms > 1000 ) {
        while( ms > 1000 ) {
            usleep( 1000 * 1000 );
            ms -= 1000;
        }
    }
    usleep( ms * 1000 );
}

void lcd_send_nibble( char n ) {
    int d0, d1, d2, d3;
    d0 = d1 = d2 = d3 = 0;
    if( n&1 ) d0 = 1;
    if( n&2 ) d1 = 1;
    if( n&4 ) d2 = 1;
    if( n&8 ) d3 = 1;
    gpio_set_value( lcd_d0, d0 );
    gpio_set_value( lcd_d1, d1 );
    gpio_set_value( lcd_d2, d2 );
    gpio_set_value( lcd_d3, d3 );
    delay_ms(1);
    gpio_set_value( lcd_en, 1 ); 
    delay_ms(1);
    gpio_set_value( lcd_en, 0 ); 
}

void lcd_send_byte( char type, char data ) {
    delay_ms( 1 );      // no busy check
    if( type == 0 ) {             // lcd cmd mode
        gpio_set_value( lcd_rs, 0 );  
    } else {                      // lcd write mode
        gpio_set_value( lcd_en, 1 );
        gpio_set_value( lcd_rs, 1 );
    }

    lcd_send_nibble( data >> 4 );
    lcd_send_nibble( data & 0xF );
}

void lcd_clear() {
    lcd_send_byte(0, CMD_CLEAR);
    delay_ms( 4 );
}

void lcd_init() {
    init_lcd_port();

    delay_ms(100);  
    gpio_set_value( lcd_rs, 0 );
    gpio_set_value( lcd_en, 0 );
    delay_ms(15);

    int i;
    // 4 special cases of function set instruction
    for(i=1;i<=3;++i) {
        lcd_send_nibble(3);
        delay_ms(5);
    }
    lcd_send_nibble(2);

    // Configure the Function Set of the LCD.   
    lcd_send_byte(0, CMD_FUNCTION_SET | MSK_DL_4 | MSK_N | MSK_F);

    // Configure the display on/off control of the LCD.
    lcd_send_byte(0, CMD_DISPLAY_CONTROL | MSK_D_OFF | MSK_C | MSK_B);

    // Clear the LCD display.
    lcd_send_byte(0, CMD_CLEAR);
    delay_ms( 4 );

    // Configure the entry mode set of the LCD.
    lcd_send_byte(0, CMD_ENTRY_MODE_SET | MSK_ID | MSK_S);

    // Configure the display on/off control of the LCD.
    lcd_send_byte(0, CMD_DISPLAY_CONTROL | MSK_D | MSK_C | MSK_B);
    delay_ms( 1 );
}


void lcd_gotoxy( char col, char row ) {
    char address;
    switch( row ) {
        case 2 : address = LCD_LINE_2_ADDR; break;
        case 3 : address = LCD_LINE_3_ADDR; break;
        case 4 : address = LCD_LINE_4_ADDR; break;
        default: address = LCD_LINE_1_ADDR; break;
    }
    address += col - 1;
    lcd_send_byte( 0, CMD_SET_DDRAM_ADDRESS | address );
}

void lcd_putc( char c ) {
    lcd_send_byte( 1, c );  
}

void lcd_puts( char * str, int str_len ) {
    int i;
    for( i = 0; i < str_len; i++ ) {
        lcd_send_byte( 1, str[ i ] );
    }
}

void lcd_putd( int num, int digit_count ) {
    int d = pow( 10, digit_count - 1 );
    int msd;

    while( d >= 1 ) {
        msd = num / d;
        lcd_send_byte( 1, msd + 48 );
        num -= msd * d;
        d /= 10;
    }
}
