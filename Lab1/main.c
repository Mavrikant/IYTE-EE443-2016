
#include <avr/io.h>
#include <util/delay.h>
int main(void){
    DDRB = 0b11111111;
    while(1){
        PORTB = 0b00001111;
        PORTB = 0b00001111;
        PORTB = 0b00001111;
        PORTB = 0b11110000;
    };
    return 0;
}
