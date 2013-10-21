
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "io_lib.h"
#include "lcd.h"

#define cursor_left     70
#define cursor_right    72
#define inc_key         74
#define dec_key         76
#define set_key         78    

typedef struct lcd_xy {
    int col;
    int row;
} lcd_xy;

typedef struct lcd_pos {
    lcd_xy t1;
    lcd_xy t2;
    lcd_xy t3;
    lcd_xy t4;
    lcd_xy t5;
    lcd_xy t6;
    lcd_xy date;
    lcd_xy month;
    lcd_xy year;
    lcd_xy hour;
    lcd_xy min;
    lcd_xy sec;
    lcd_xy int_hour;
    lcd_xy int_min;
    lcd_xy log_count;
} lcd_pos;   

lcd_pos pos = {  }


double ohms_to_celsius( double ohms ) {
    double beta = 3930; // ( approx value, see datasheet for accurate value )
    double temperature_inv = ( 1 / 298.15 ) + ( 1 / beta ) * ( log( ohms / 10000 ) );
    return 1 / temperature_inv - 273.15;
}

double adc_mv_to_ohms( int adc_mv ) {
    double adc_v = adc_mv / 1000;
    return ( 18.25 * adc_v ) / ( 9.125 - adc_v * 8.65 );
}

double adc_mv_to_celsius( int adc_mv ) {
    double ohms = adc_mv_to_ohms( adc_mv );
    return ohms_to_celsius( ohms );
}

double read_temperature( int index ) {
    char adc_file[40];
    char adc_str[10];

    sprintf( adc_file, "/sys/devices/ocp.2/helper.14/AIN%d", index );
    read_val( adc_file, adc_str );
    return adc_mv_to_celsius( atoi( adc_str ) );
}

void fill_buffer( char * write_buffer, time_t cur_time ) {
    char temperature[10];
    char time_str[30];
    sprintf( write_buffer, "\n" );
    int i;
    for(i = 0; i < 6; i++ ) {
        sprintf( temperature, "%.2f\t", read_temperature( i ) );
        strcat( write_buffer, temperature );
    }
    sprintf( time_str, "\t%s", ctime( &cur_time ) );
    strcat( write_buffer, time_str );
}

void usb_device_write() {
    system("if [ -d \"/media/USB20FD\" ]; then cp temperature_log.txt /media/USB20FD; else echo \"usb drive not inserted properly\"; fi");
}

int is_key_pressed( int key ) {
    if( gpio_get_value( key ) == 0 ) {
        delay_ms( 200 );
        if( gpio_get_value( key ) == 0 )
            return 1;
    }
    return 0;
}

void set_rtc_time( struct tm * timeinfo; ) {
    time_t set_time = mktime ( timeinfo );
    stime( &set_time );
    system( "hwclock -w -f /dev/rtc1" );
}

void set_mode() {

}

void lcd_write() {
    lcd_gotoxy( 1, 1 );
    lcd_puts( "abcdef ", 7 );
    lcd_gotoxy( 1, 2 );
    lcd_putd( 123456, 6 );
}

int is_time_to_write( time_t cur_time, time_t last_log_time, int temp_log_interval_min ) {
    if( cur_time - last_log_time >= temp_log_interval_min * 60 )
        return 1;
    return 0;
}

void input_btn_init() {
    init_gpio( cursor_left );
    init_gpio( cursor_right );
    init_gpio( inc_key );
    init_gpio( dec_key );
    init_gpio( set_key );
    
    gpio_direction_in( cursor_left );
    gpio_direction_in( cursor_right );
    gpio_direction_in( inc_key );
    gpio_direction_in( dec_key );
    gpio_direction_in( set_key );
}

void init() {
    input_btn_init();
    lcd_init();
}

int main() {
    init();
    
    char log_file[] = "temperature_log.txt";
    char write_buffer[ 200 ];
    time_t cur_time, last_log_time;
    time( &cur_time );
    last_log_time = cur_time;
    
    while( 1 ) {
        time( &cur_time );
        if( is_time_to_write( cur_time, last_log_time, temp_log_interval_min ) ) {
            fill_buffer( write_buffer, cur_time );
            FILE *fout = fopen( log_file, "a+" );
            if( fout != NULL ) {
                fwrite(write_buffer, strlen(write_buffer), 1, fout);
                fclose( fout );
                sync();
            }
            printf( "%s\n", write_buffer );
            last_log_time = cur_time;
        }
        if( is_key_pressed( set_key ) )
            usb_device_write();
        else if( is_key_pressed( cursor_left ) || is_key_pressed( cursor_right ) )
            set_mode();
    }
    
    return 0;
}
