
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "io_lib.c"
#include "lcd.c"


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

void read_temperature( double * temperature_values ) {
    char adc_path[40] = "/sys/devices/ocp.2/helper.14/";
    char adc_files[8][5] = { "AIN0", "AIN1", "AIN2", "AIN3", "AIN4", "AIN5", "AIN6", "AIN7" };
    char adc_file[40];
    char adc_str[10];
    
    int i;
    printf("\n adc_val : ");
    for( i = 0; i < 8; i++ ) {
        strcpy( adc_file, adc_path );
        strcat( adc_file, adc_files[i] );
        read_val( adc_file, adc_str );
        temperature_values[i] = adc_mv_to_celsius( atoi( adc_str ) );
        printf("%d  %f;\t", atoi( adc_str ), temperature_values[i] );
    }
    printf("\n");
}

void fill_buffer( char * write_buffer, time_t cur_time ) {
    double temperature_values[8] = { 0 };
    char temperature[10];
    char time_str[30];
    read_temperature( temperature_values );
    sprintf( write_buffer, "\n" );
    int i;
    for(i = 0; i < 8; i++ ) {
        sprintf( temperature, "%.2f\t", temperature_values[i] );
        strcat( write_buffer, temperature );
    }
    sprintf( time_str, "\t%s", ctime( &cur_time ) );
    strcat( write_buffer, time_str );
}

void usb_device_write() {
    system("cp temperature_log.txt /media/USB20FD");
}

int is_key_pressed() {
    
    return 0;
}

void lcd_write() {
    lcd_gotoxy( 1, 1 );
    lcd_puts( "abcdef ", 7 );
    lcd_gotoxy( 1, 2 );
    lcd_putd( 6, 123456 );
}

void init() {
    lcd_init();
    lcd_write();
}

int main() {
    init();
    
    char log_file[] = "temperature_log.txt";
    int i, last_log_min;
    char write_buffer[ 200 ];
    time_t cur_time;
    struct tm * timeinfo;
    time( &cur_time );
    timeinfo = localtime (&cur_time);
    last_log_min = timeinfo->tm_min;
    
    while( 1 ) {
        time( &cur_time );
        timeinfo = localtime (&cur_time);
        if( timeinfo->tm_min % 1 == 0 && timeinfo->tm_min != last_log_min ) {
            fill_buffer( write_buffer, cur_time );
            FILE *fout = fopen( "temperature_log.txt", "a+" );
            fwrite(write_buffer, strlen(write_buffer), 1, fout);
            fclose( fout );
            sync();
            printf( "%s\n", write_buffer );
            last_log_min = timeinfo->tm_min;
        }
        if( is_key_pressed() )
            usb_device_write();

        lcd_write();
    }
    
    return 0;
}
