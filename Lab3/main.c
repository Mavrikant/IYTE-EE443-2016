#include <avr/io.h>
#include <util/delay.h>
#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)


void mydelay(void){
    int i = 0;
    for (i = 0; i < 1000; i++) {
        __asm__ volatile ( "NOP" );
    }
}

void change (void){
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
            PORTB= PORTB << 1;//shift left (1 clock)
        }
    }

}

int main(void){
    DDRB |= 0b11111111;// all output
    DDRD |= 0b11111100;//0-1 input, others  output
    PORTD |= 0b00000011;//Pull-up resistor for inputs
    PORTB = 0b00010000; //initiate
    unsigned int PDsave = 0x00;
    unsigned int PDsave_old = 0x00;
    int count = 0;

    while(1){
        PDsave_old = PDsave;
        PDsave = PIND;

        if(((PDsave_old & 0x01) == 0x01) && ((PDsave & 0x01) == 0x00)){ // 1 to 0 transition at SW0
            change();
            count = 0;
        }
        else if (((PDsave_old & 0x01) == 0x00) && ((PDsave & 0x01) == 0x00)){ // no change 0 - 0
           if (count>250){//wait until 250ms
                change();
                count = 0;
           }
           count++;
        }

        mydelay();//1ms

    }

    return 0;
}
