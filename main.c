
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
    lcd_xy date;
    lcd_xy month;
    lcd_xy year;
    lcd_xy hour;
    lcd_xy min;
    lcd_xy sec;
    lcd_xy hour_intrvl;
    lcd_xy min_intrvl;
    lcd_xy log_count;
} lcd_pos;   

lcd_pos data_pos =  {   .date = { 1, 3 },   .month = { 4, 3 },  .year = { 7, 3 }, 
                        .hour = { 13, 3 },  .min = { 16, 3 },   .sec = { 19, 3 },
                        .hour_intrvl={5,4}, .min_intrvl={8,4},  .log_count={15,4}
                    };

int hour_intrvl = 0, min_intrvl = 1, log_count = 0;


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

    sprintf( adc_file, "/sys/devices/ocp.2/helper.11/AIN%d", index );
    read_val( adc_file, adc_str );
    double temperature = adc_mv_to_celsius( atoi( adc_str ) );
    if( temperature < -99 ) temperature = -99;
    else if( temperature > 999 ) temperature = 999;
    return temperature;
}

void fill_buffer( char * write_buffer, time_t cur_time ) {
    char temperature[10];
    char time_str[30];
    sprintf( write_buffer, "\n" );
    int i;
    for(i = 0; i < 6; i++ ) {
        sprintf( temperature, "%4.1f\t", read_temperature( i ) );
        strcat( write_buffer, temperature );
    }
    sprintf( time_str, "\t%s", ctime( &cur_time ) );
    strcat( write_buffer, time_str );
}

void usb_device_write() {
    system("if [ -d \"/media/USB20FD\" ]; then cp temperature_log.txt /media/USB20FD; else echo \"usb drive not inserted properly\"; fi");
}

int is_key_pressed( int key ) {
    return 0;
    if( gpio_get_value( key ) == 0 ) {
        delay_ms( 200 );
        if( gpio_get_value( key ) == 0 ) return 1;
    }
    return 0;
}

void set_rtc_time( struct tm * timeinfo ) {
    time_t set_time = mktime ( timeinfo );
    stime( &set_time );
    system( "hwclock -w -f /dev/rtc1" );
}

int change_disp_val( int val, int max_val, int min_val, int pos_col, int pos_row, int no_of_digits ) {
    while( 1 ) {
        if( is_key_pressed( inc_key ) ) {
            if( val >= max_val ) val = 1;
            else val++;
            lcd_gotoxy( pos_col, pos_row );
            lcd_putd( val, no_of_digits );
        } else if( is_key_pressed( dec_key ) ) {
            if( val <= min_val ) val = max_val;
            else val--;
            lcd_gotoxy( pos_col, pos_row );
            lcd_putd( val, no_of_digits );
        } else if( is_key_pressed( set_key ) || is_key_pressed( cursor_left ) || is_key_pressed( cursor_right ) )
            break;
    }
    return val;
}

void set_time_val( char c, int val ) {
    time_t cur_time;
    struct tm * timeinfo;
    time( &cur_time );
    timeinfo = localtime( &cur_time );
    switch( c ) {
        case 'd':
            timeinfo->tm_mday = val; break;
        case 'm':
            timeinfo->tm_mon = val; break;
        case 'y':
            timeinfo->tm_year = val; break;
        case 'h':
            timeinfo->tm_hour = val; break;
        case 'n':
            timeinfo->tm_min = val; break;
        case 's':
            timeinfo->tm_sec = val; break;
    }
    set_rtc_time( timeinfo );
}

void set_mode() {
    int val;
    time_t cur_time;
    struct tm * timeinfo;

    time( &cur_time );
    timeinfo = localtime( &cur_time );
    val = change_disp_val( timeinfo->tm_mday, 31, 1, data_pos.date.col, data_pos.date.row, 2 );
    if( val != timeinfo->tm_mday ) set_time_val( 'd', val );

    time( &cur_time );
    timeinfo = localtime( &cur_time );
    val = change_disp_val( timeinfo->tm_mon, 11, 0, data_pos.month.col, data_pos.month.row, 2 );
    if( val != timeinfo->tm_mon ) set_time_val( 'm', val );

    time( &cur_time );
    timeinfo = localtime( &cur_time );
    val = change_disp_val( timeinfo->tm_year, 2100, 2000, data_pos.year.col, data_pos.year.row, 4 );
    if( val != timeinfo->tm_year ) set_time_val( 'y', val );

    time( &cur_time );
    timeinfo = localtime( &cur_time );
    val = change_disp_val( timeinfo->tm_hour, 23, 0, data_pos.hour.col, data_pos.hour.row, 2 );
    if( val != timeinfo->tm_hour ) set_time_val( 'h', val );

    time( &cur_time );
    timeinfo = localtime( &cur_time );
    val = change_disp_val( timeinfo->tm_min, 59, 0, data_pos.min.col, data_pos.min.row, 2 );
    if( val != timeinfo->tm_min ) set_time_val( 'n', val );

    time( &cur_time );
    timeinfo = localtime( &cur_time );
    val = change_disp_val( timeinfo->tm_sec, 59, 0, data_pos.sec.col, data_pos.sec.row, 2 );
    if( val != timeinfo->tm_sec ) set_time_val( 's', val );

    hour_intrvl = change_disp_val( hour_intrvl, 99, 0, data_pos.hour_intrvl.col, data_pos.hour_intrvl.row, 2 );
    min_intrvl = change_disp_val( min_intrvl, 99, 0, data_pos.min_intrvl.col, data_pos.min_intrvl.row, 2 );
    log_count = change_disp_val( log_count, 99, 0, data_pos.log_count.col, data_pos.log_count.row, 2 );
}

int access_data( FILE * fp, int pos, int data_len ) {
    char data_str[10];
    fseek( fp, pos, SEEK_SET );
    fgets( data_str, data_len, fp );
    return atoi( data_str );
}

void store_data( int hour, int min, int count ) {
    printf( "store_data\n" );
    int max_data_len = 9;
    int hour_pos = 1, min_pos = hour_pos + max_data_len, count_pos = min_pos + max_data_len;
    FILE * data_store = data_store = fopen( "data_store.txt", "w" );
    if( data_store != NULL ) {
        fseek( data_store, hour_pos, SEEK_SET );
        fprintf( data_store, "%d", hour_intrvl );
        fseek( data_store, min_pos, SEEK_SET );
        fprintf( data_store, "%d", min_intrvl );
        fseek( data_store, count_pos, SEEK_SET );
        fprintf( data_store, "%d", log_count );
        fclose( data_store );
    } else {
        printf( "could not create file data_store\n" );
    }
}

void read_stored_data() {
    printf( "read_stored_data\n" );
    FILE * data_store = fopen( "data_store.txt", "r+" );
    int max_data_len = 9;
    int hour_pos = 1, min_pos = hour_pos + max_data_len, count_pos = min_pos + max_data_len;
    if( data_store != NULL ) {
        hour_intrvl = access_data( data_store, hour_pos, max_data_len );
        min_intrvl = access_data( data_store, min_pos, max_data_len );
        log_count = access_data( data_store, count_pos, max_data_len );
        fclose( data_store );
    } else {
        hour_intrvl = 0; min_intrvl = 1; log_count = 0;
        store_data( hour_intrvl, min_intrvl, log_count );
    }
}

void fill_line_str( char * cur_line, int line_no ) {
    if( line_no == 1 )
        sprintf( cur_line, "%3.1f  %3.1f  %3.1f ", read_temperature(0), read_temperature(1), read_temperature(2) );
    else if( line_no == 2 )
        sprintf( cur_line, "%3.1f  %3.1f  %3.1f ", read_temperature(3), read_temperature(4), read_temperature(5) );
    else if( line_no == 3 ) {
        time_t cur_time;
        time( &cur_time );
        struct tm * timeinfo = localtime( &cur_time );
        sprintf( cur_line, "%02d/%02d/%4d  %02d:%02d:%02d", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year+1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec );
    } else if( line_no == 4 ) {
        sprintf( cur_line, "INT:%02d:%02d CNT:%06d", hour_intrvl, min_intrvl, log_count );
    }
}

void display_data_lcd() {
    char cur_line[21];
    int line_no = 1;
    for( line_no = 1; line_no <= 4; line_no++ ) {
        fill_line_str( cur_line, line_no );
        lcd_gotoxy( 1, line_no );
        lcd_puts( cur_line, 20 );
    }
}

int is_time_to_write( time_t cur_time, time_t last_log_time ) {
    if( cur_time - last_log_time >= hour_intrvl * 3600 + min_intrvl * 60 )
        return 1;
    return 0;
}

int log_temperature( time_t cur_time ) {
    char log_file[] = "temperature_log.txt";
    char write_buffer[ 200 ];
    fill_buffer( write_buffer, cur_time );
    FILE *fout = fopen( log_file, "a+" );
    if( fout != NULL ) {
        fwrite(write_buffer, strlen(write_buffer), 1, fout);
        fclose( fout );
        sync();
        printf( "%s\n", write_buffer );
        return 1;
    }
    return 0;
}

void input_key_init() {
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
    input_key_init();
    lcd_init();
    read_stored_data();
}

int main() {
    init();

    int iter_count = 0;
    time_t cur_time, last_log_time;
    time( &cur_time );
    last_log_time = cur_time;
    
    while( 1 ) {
        iter_count++;
        time( &cur_time );
        if( is_time_to_write( cur_time, last_log_time ) ) {
            if( log_temperature( cur_time ) ) {
                log_count++;
                store_data( hour_intrvl, min_intrvl, log_count );
            }
            last_log_time = cur_time;
        }
        if( is_key_pressed( set_key ) )
            usb_device_write();
        else if( is_key_pressed( cursor_left ) || is_key_pressed( cursor_right ) )
            set_mode();
        if( iter_count > 100000 ) {
            display_data_lcd();
            iter_count = 0;
        }
    }
    
    return 0;
}
