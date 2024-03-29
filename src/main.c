
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <mntent.h>

#include "io_lib.h"
#include "lcd.h"

#define cursor_left     72
#define cursor_right    74
#define inc_key         76
#define dec_key         78
#define set_key         8  

#define max_log_count   10000

#define log_path        "/home/root/temperature_log.txt"
#define old_log_path    "/home/root/temperature_log_old.txt"
#define data_store_path "/home/root/data_store.txt"
#define usb_drive       "/media/usb_device"

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

int prev_adc_mv[6] = {0};

//resistance of thermistors at 25.1 celsius
double res_at_25_1[ 6 ] = { 10840, 9900, 10280, 9790, 9450, 9480 };

double ohms_to_celsius( double ohms, int index ) {
    double beta = 3480; // ( see thermistor datasheet for value )
    double temperature_inv = ( 1 / (273.15 + 25.1) ) + ( 1 / beta ) * ( log( ohms / res_at_25_1[ index ] ) );
    return ( 1 / temperature_inv ) - 273.15;
}


double get_thermistor_resistance( int adc_mv ) {
    double vout = (double)adc_mv / 1000;    // output voltage of voltage divider circuit used
    double vin = 2.5;               // input voltage of voltage divider
    double rpot = 27100;            // resistance of potentiometer 
    return vout * rpot / ( vin - vout );
}

double adc_mv_to_celsius( int adc_mv, int index ) {
    double rth = get_thermistor_resistance( adc_mv );
    printf("adc = %d\tth_res = %f\t", adc_mv, rth);
    return ohms_to_celsius( rth, index );
}

void build_path( char * path, char * dir_to_find, int dir_char_count ) {
    DIR *dp;
    struct dirent *ep;     
    dp = opendir ( path );
    if (dp != NULL) {
        while (ep = readdir (dp)) {
            if( strncmp( ep->d_name, dir_to_find, dir_char_count ) == 0 ) {
                sprintf( path, "%s/%s", path, ep->d_name );
                (void) closedir (dp);
                return;
            }
        }
        (void) closedir (dp);
    }
    else
        perror ("Couldn't open the directory");
}

void get_adc_path( char * adc_path, int index ) {
    sprintf( adc_path, "/sys/devices" );
    build_path( adc_path, "ocp", 3 );
    build_path( adc_path, "helper", 6 );
    sprintf( adc_path, "%s/AIN%d", adc_path, index );
}

int average_adc( int index ) {
    char adc_path[40];
    char adc_str[10];
    int adc_mv;
    int adc_mv_135_celsius = 40;
    int adc_mv_sub_zero = 1700;
    int i = 0;
    if( index == 0 ) index = 6; // AIN6 used instead of AIN0
    get_adc_path( adc_path, index );

    do {
        read_val( adc_path, adc_str );
        adc_mv = atoi(adc_str);
        delay_ms( 500 );    
    } while( i++ < 5 && ( adc_mv < adc_mv_135_celsius || adc_mv > adc_mv_sub_zero ));

    if( adc_mv < adc_mv_135_celsius || adc_mv > adc_mv_sub_zero ) {
        return prev_adc_mv[index];
    } else {
        prev_adc_mv[index] = adc_mv;
        return adc_mv;
    }
}

double read_temperature( int index ) {
    int adc_mv = average_adc( index );
    printf("\nindex: %d\t", index, adc_mv);
    double temperature = adc_mv_to_celsius( adc_mv, index );
    printf("temperature = %f\n", temperature);
    if( temperature < -10 ) temperature = -10;
    else if( temperature > 125 ) temperature = 125;
    return temperature;
}

void create_new_log_file() {
    remove( old_log_path );
    rename( log_path, old_log_path );
}

int is_mounted (char * mount_dir) {
    FILE * mtab = NULL;
    struct mntent * part = NULL;
    if( ( mtab = setmntent( "/etc/mtab", "r" ) ) != NULL ) {
        while( ( part = getmntent( mtab ) ) != NULL ) {
            if( part->mnt_dir != NULL && strcmp( part->mnt_dir, mount_dir ) == 0 ) {
                endmntent( mtab );
                return 1;
            }
        }
    }
    return 0;
}

void usb_device_write() {
    char cmd[200];
    if( is_mounted( usb_drive ) ) {
        lcd_gotoxy( 1, 4 );
        lcd_puts( "Transferring Data...", 20 );
        sprintf( cmd, "cp %s %s", log_path, usb_drive );
        system(cmd);
        delay_ms( 2000 );
        sprintf( cmd, "cp %s %s", old_log_path, usb_drive );
        system(cmd);
        delay_ms( 2000 );
        sprintf( cmd, "umount %s", usb_drive );
        system(cmd);
        lcd_gotoxy( 1, 4 );
        lcd_puts( "Data transfer done  ", 20 );
        delay_ms( 2000 );
    } else {
        lcd_gotoxy( 1, 4 );
        lcd_puts( "Re-insert USB device", 20 );
        delay_ms( 2000 );
    }
}

int is_key_pressed( int key ) {
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
    int iter_count = 0;
    while( 1 ) {
        lcd_gotoxy( pos_col, pos_row );
        lcd_putd( val, no_of_digits );
        delay_ms(200);
        while ( is_key_pressed( inc_key ) ) {
            if( max_val == min_val )    val = max_val;
            else if( val >= max_val )   val = min_val;
            else val++;
            lcd_gotoxy( pos_col, pos_row );
            lcd_putd( val, no_of_digits );
        }
        while ( is_key_pressed( dec_key ) ) {
            if( max_val == min_val )    val = max_val;
            else if( val <= min_val )   val = max_val;
            else val--;
            lcd_gotoxy( pos_col, pos_row );
            lcd_putd( val, no_of_digits );
        } 
        if( iter_count > 2 && ( is_key_pressed( cursor_left ) || is_key_pressed( cursor_right ) ) )
            break;

        lcd_gotoxy( pos_col, pos_row );
        int i;
        for( i = 0; i < no_of_digits; i++ )
            lcd_putc( ' ' );
        delay_ms(100);
        iter_count++;
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

void store_data() {
    int max_data_len = 9;
    int hour_pos = 1, min_pos = hour_pos + max_data_len, count_pos = min_pos + max_data_len;
    FILE * data_store = data_store = fopen( data_store_path, "w" );
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

void set_mode( char direction ) {
    static cur_disp_index = 0;
    int max_disp_index = 9, min_disp_index = 1;
    int val, prev_val;
    time_t cur_time;
    struct tm * timeinfo;

    if( direction == 'r' ) {
        if( cur_disp_index == max_disp_index ) cur_disp_index = min_disp_index;
        else cur_disp_index++;
    }
    else {
        if( cur_disp_index == min_disp_index ) cur_disp_index = max_disp_index;
        else cur_disp_index--;
    }

    time( &cur_time );
    timeinfo = localtime( &cur_time );
    switch( cur_disp_index ) {
        case 1:
            val = change_disp_val( timeinfo->tm_mday, 31, 1, data_pos.date.col, data_pos.date.row, 2 );
            if( val != timeinfo->tm_mday ) set_time_val( 'd', val );
            break;
        case 2:
            val = change_disp_val( timeinfo->tm_mon+1, 12, 1, data_pos.month.col, data_pos.month.row, 2 );
            if( val != timeinfo->tm_mon+1 ) set_time_val( 'm', val-1 );
            break;
        case 3:
            val = change_disp_val( timeinfo->tm_year+1900, 2100, 2000, data_pos.year.col, data_pos.year.row, 4 );
            if( val != timeinfo->tm_year+1900 ) set_time_val( 'y', val-1900 );
            break;
        case 4:
            val = change_disp_val( timeinfo->tm_hour, 23, 0, data_pos.hour.col, data_pos.hour.row, 2 );
            if( val != timeinfo->tm_hour ) set_time_val( 'h', val );
            break;
        case 5:
            val = change_disp_val( timeinfo->tm_min, 59, 0, data_pos.min.col, data_pos.min.row, 2 );
            if( val != timeinfo->tm_min ) set_time_val( 'n', val );
            break;
        case 6:
            val = change_disp_val( timeinfo->tm_sec, 59, 0, data_pos.sec.col, data_pos.sec.row, 2 );
            if( val != timeinfo->tm_sec ) set_time_val( 's', val );
            break;
        case 7:
            prev_val = hour_intrvl;
            hour_intrvl = change_disp_val( hour_intrvl, 99, 0, data_pos.hour_intrvl.col, data_pos.hour_intrvl.row, 2 );
            if( prev_val != hour_intrvl )   store_data();
            break;
        case 8:
            prev_val = min_intrvl;
            min_intrvl = change_disp_val( min_intrvl, 59, 0, data_pos.min_intrvl.col, data_pos.min_intrvl.row, 2 );
            if( prev_val != min_intrvl )   store_data();
            break;
        case 9:
            log_count = change_disp_val( log_count, 0, 0, data_pos.log_count.col, data_pos.log_count.row, 6 );
            if( log_count == 0 ) {
                create_new_log_file();
                store_data();
            }
            break;
    }
}

void fill_line_str( char * cur_line, int line_no ) {
    if( line_no == 1 )
        sprintf( cur_line, "%5.1f  %5.1f  %5.1f ", read_temperature(0), read_temperature(1), read_temperature(2) );
    else if( line_no == 2 )
        sprintf( cur_line, "%5.1f  %5.1f  %5.1f ", read_temperature(3), read_temperature(4), read_temperature(5) );
    else if( line_no == 3 ) {
        time_t cur_time;
        time( &cur_time );
        struct tm * timeinfo = localtime( &cur_time );
        sprintf( cur_line, "%02d/%02d/%4d  %02d:%02d:%02d", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec );
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

int is_time_to_log( time_t cur_time, time_t last_log_time ) {
    time_t intrvl_time = hour_intrvl * 3600 + min_intrvl * 60;
    if( intrvl_time > 0 && cur_time - last_log_time >= intrvl_time )
        return 1;
    return 0;
}


void fill_log_buffer( char * log_buffer, time_t cur_time ) {
    char temperature[10];
    char time_str[30];
    sprintf( log_buffer, "\n%5d:\t", log_count + 1 );
    int i;
    for(i = 0; i < 6; i++ ) {
        sprintf( temperature, "%4.1f\t", read_temperature( i ) );
        strcat( log_buffer, temperature );
    }
    sprintf( time_str, "\t%s", ctime( &cur_time ) );
    strcat( log_buffer, time_str );
}

int log_temperature( time_t cur_time ) {
    char log_buffer[ 200 ];
    fill_log_buffer( log_buffer, cur_time );
    FILE *f_log = fopen( log_path, "a+" );
    if( f_log != NULL ) {
        fwrite(log_buffer, strlen(log_buffer), 1, f_log);
        fclose( f_log );
        sync();
        printf( "%s\n", log_buffer );
        return 1;
    }
    return 0;
}

int get_stored_data( FILE * fp, int pos, int data_len ) {
    char data_str[10];
    fseek( fp, pos, SEEK_SET );
    fgets( data_str, data_len, fp );
    return atoi( data_str );
}

void read_stored_data() {
    printf( "read_stored_data\n" );
    FILE * data_store = fopen( data_store_path, "r+" );
    int max_data_len = 9;
    int hour_pos = 1, min_pos = hour_pos + max_data_len, count_pos = min_pos + max_data_len;
    if( data_store != NULL ) {
        hour_intrvl = get_stored_data( data_store, hour_pos, max_data_len );
        min_intrvl = get_stored_data( data_store, min_pos, max_data_len );
        log_count = get_stored_data( data_store, count_pos, max_data_len );
        fclose( data_store );
    } else {
        hour_intrvl = 0; min_intrvl = 1; log_count = 0;
        store_data();
    }
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
        if( is_time_to_log( cur_time, last_log_time ) ) {
            if( log_temperature( cur_time ) ) {
                log_count++;
                store_data();
                last_log_time = cur_time;
                if( log_count >= max_log_count ) {
                    create_new_log_file();
                    log_count = 0;
                }
            }
        }
        if( is_key_pressed( set_key ) )
            usb_device_write();
        else if( is_key_pressed( cursor_left ) )
            set_mode( 'l' );
        else if ( is_key_pressed( cursor_right ) )
            set_mode( 'r' );
        if( iter_count > 1000 ) {
            display_data_lcd();
            iter_count = 0;
        }
    }
    return 0;
}
