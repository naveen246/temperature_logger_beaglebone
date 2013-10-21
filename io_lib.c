
#include "io_lib.h"

char * gpio_path = "/sys/class/gpio/";

void read_val( char * filename, char * val ) {
    FILE *fp = fopen( filename, "r" );
    if( fp != NULL ) {
        fscanf( fp, "%s", val );
        fclose( fp );
    }
}

void write_val( char * filename, char * val ) {
    FILE *fp = fopen( filename, "w" );
    if( fp != NULL ) {
        fprintf( fp, "%s", val );
        fclose( fp );
    }
}

void init_gpio( int gpio_no ) {
    char gpio_export[40];
    sprintf( gpio_export, "%sexport", gpio_path );
    char gpio_str[5];
    sprintf(gpio_str,"%d",gpio_no);
    write_val( gpio_export, gpio_str );
}

void set_gpio_direction( int gpio_no, char * dir ) {
    char gpio_pin_dir[50];
    sprintf( gpio_pin_dir, "%sgpio%d/direction", gpio_path, gpio_no );
    write_val( gpio_pin_dir, dir );
}

void gpio_direction_in( int gpio_no ) {
    set_gpio_direction( gpio_no, "in" );
}

void gpio_direction_out( int gpio_no ) {
    set_gpio_direction( gpio_no, "out" );
}

	
void gpio_set_value( int gpio_no, int value ) {
    char gpio_pin_val[50];
    sprintf( gpio_pin_val, "%sgpio%d/value", gpio_path, gpio_no );
    char val_str[5];
    if( value != 0 && value != 1 ) value = 1;
    sprintf(val_str,"%d",value);
    write_val( gpio_pin_val, val_str );
}

int gpio_get_value( int gpio_no ) {
    char gpio_pin_val[50];
    sprintf( gpio_pin_val, "%sgpio%d/value", gpio_path, gpio_no );
    char val_str[5];
    read_val( gpio_pin_val, val_str );
    return atoi( val_str );
}




