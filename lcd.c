
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

#define lcd_rs  65
//#define lcd_rw  PIN_?
#define lcd_en  27

#define lcd_d0  47
#define lcd_d1  26
#define lcd_d2  44
#define lcd_d3  45

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

/* port specific elements end */

/* lcd functions */

void delay_ms( int ms ) {
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
   //output_high( lcd_pwr );
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

void lcd_putd( int digit_count, int num ) {
    int d = pow( 10, digit_count - 1 );
    int msd;

    while( d >= 1 ) {
        msd = num / d;
        lcd_send_byte( 1, msd + 48 );
        num -= msd * d;
        d /= 10;
    }
}
