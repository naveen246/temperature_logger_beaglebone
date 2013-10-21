
#ifndef IO_LIB_H_
#define IO_LIB_H_

extern char * gpio_path;

void read_val( char * filename, char * val );

void write_val( char * filename, char * val );

void init_gpio( int gpio_no );

void set_gpio_direction( int gpio_no, char * dir );

void gpio_direction_in( int gpio_no );

void gpio_direction_out( int gpio_no );
	
void gpio_set_value( int gpio_no, int value );

int gpio_get_value( int gpio_no );


#endif //IO_LIB_H_

