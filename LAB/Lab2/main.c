
#include <avr/io.h>
#include <util/delay.h>
#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)


int main(void){

    DDRB  = 0b11111111;//output for LEDs on Port B
    DDRD  = 0b11111100; //Pin 0,1 are input for PortD
    PORTD = 0b00000011;//Pull up resistors are enabled for pin 0,1 at PortD
    PORTB = 0b00000001; //Starting from right

   while(1){
        if(( PIND & _BV(PIND0) ) == 0x00){ //SW0 pressed
            if (( PIND & _BV(PIND1) ) == 0x00){ //SW1  and SW0 pressed (1 clock)
                _NOP(); // to make right and left shift time equal.
                if(PORTB == 0b00000001){ //(1 clock)
                    _NOP();
                    PORTB = 0b10000000;//Turn to start (1 clock)
                }
                else{// + jump (1 clock)
                    PORTB= PORTB >> 1;;//shift right (1 clock))
                }

            }
            else{ //SW1 not pressed, SW0 pressed  + jump (1 clock)

                if(PORTB == 0b10000000){ //(1 clock)
                    _NOP();
                    PORTB = 0b00000001; //Turn to end (1 clock)
                }
                else{// + jump (1 clock)
                    PORTB= PORTB << 1; //shift left (1 clock)
                }
            }
        }
   }

return 0;
}

