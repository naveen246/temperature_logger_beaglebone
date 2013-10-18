// The command code for the LCD.
#define CMD_CLEAR                0b00000001   // Clear the LCD display.
#define CMD_HOME                 0b00000010   // Return to home.

// The maskable command code to change the configuration of the LCD.
#define CMD_ENTRY_MODE_SET       0b00000100   // Bit: 0  0  0  0  0  1  ID S
#define CMD_DISPLAY_CONTROL      0b00001000   // Bit: 0  0  0  0  1  D  C  B
#define CMD_FUNCTION_SET         0b00100000   // Bit: 0  0  1  DL N  F  0  0
                              
// The mask to change the configuration of the LCD.
#define MSK_ID                   0b00000010   // Increment.
#define MSK_S                    0b00000000   // Don't shift.
#define MSK_D                    0b00000100   // On display.
#define MSK_D_OFF                0b00000000   // Off display.
#define MSK_C                    0b00000000   // Don't display cursor.
#define MSK_B                    0b00000000   // Don't blink.
#define MSK_DL_8                 0b00010000   // Select 8-bit data bus.
#define MSK_DL_4                 0b00000000   // Select 4-bit data bus.
#define MSK_N                    0b00001000   // Select 2 or more-line display.
#define MSK_F                    0b00000000   // Select 5 x 8 dots character.

// The maskable command to change the LCD RAM address.
#define CMD_SET_DDRAM_ADDRESS    0b10000000   // Bit 0 - 7: Address

// The DDRAM address corresponding to 20 * 4 LCD.
#define LCD_LINE_1_ADDR          0
#define LCD_LINE_2_ADDR          0x40
#define LCD_LINE_3_ADDR          0x14
#define LCD_LINE_4_ADDR          0x54

/* port specific elements */

//#define lcd_pwr PIN_?
#define lcd_rs  PIN_B4
//#define lcd_rw  PIN_?
#define lcd_en  PIN_B5

#define lcd_d0  PIN_B0
#define lcd_d1  PIN_B1
#define lcd_d2  PIN_B2
#define lcd_d3  PIN_B3

void set_lcd_port() {
   set_tris_B( 0x00 );    
}

/* port specific elements end */

/* lcd functions */

void lcd_send_nibble( BYTE n ) {
   output_bit( lcd_d0, (n&1) );
   output_bit( lcd_d1, (n&2) );
   output_bit( lcd_d2, (n&4) );
   output_bit( lcd_d3, (n&8) );
   delay_cycles(1); 
   output_high(lcd_en); 
   delay_us(20); 
   output_low(lcd_en); 
}

void lcd_send_byte( BYTE type, BYTE data ) {
   delay_ms( 1 );      // no busy check
   if( type == 0 ) {             // lcd cmd mode
      output_low( lcd_rs );  
      //output_low( lcd_rw );
   } else {                      // lcd write mode
      output_high(lcd_en);
      output_high(lcd_rs);
      //output_low( lcd_rw );
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
   set_lcd_port();

   delay_ms(100);  
   output_low(lcd_rs); 
   //output_low(lcd_rw); 
   output_low(lcd_en); 
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


void lcd_gotoxy( BYTE col, BYTE row ) {
   BYTE address;
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

int32 pow( base, exp ) {
   int i;
   int32 res = 1;
   for( i = 1; i <= exp; i++ ) {
      res *= base;
   }
   return res;
}

void lcd_putd( int digit_count, int16 num ) {
   int32 d = pow( 10, digit_count - 1 );
   int32 msd;

   while( d >= 1 ) {
      msd = num / d;
      lcd_send_byte( 1, msd + 48 );
      num -= msd * d;
      d /= 10;
   }
}