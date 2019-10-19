#include <avr/io.h>
#include <util/delay.h>

int main(void)
{

    DDRB = 0b11111111;//output for LEDs on Port B
    DDRD = 0b11111100; //Pin 0,1 are input for PortD
    PORTD = 0b00000011;//Pull up resistors are enabled for pin 0,1 at PortD


    while(1){

        if(( PIND & _BV(PIND0) ) == 0x00){
            PORTB |= 0b00001111;
        }
        else{
            PORTB &= 0b11110000;
        }

        if(( PIND & _BV(PIND1) ) == 0x00){
            PORTB |= 0b11110000;
        }
        else{
            PORTB &= 0b00001111;
        }

    }
return 0;
}
