#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "LCDmodule.h"
#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)



int main(){
    DDRB  |= 0b11111111;// all output
    DDRD  |= 0b11111100;//0-1 input, others  output
    PORTD |= 0b00000011;//Pull-up resistor for inputs
    LCD_Init();
    LCD_Clear();

    unsigned char PDsave = 0x00;
    unsigned char PDsave_1 = 0x00;

    char LCDtext[16];
    unsigned char Atten;
    unsigned char Atten_save;

    Atten = 4;

    PrintByte(LCDtext, "Atten=");
    LCD_MoveCursor(1,1);
    LCD_WriteString(LCDtext);

    while(1){

        PDsave_1=PDsave;
        PDsave = PIND;
        Atten_save=Atten;

            if(((PIND & 0x01) == 0x00) && ((PDsave_1&0x01) == 0x01)){//SW0  pressed
                _NOP();
                _NOP();
                if(Atten > 1){
                    _NOP();
                    Atten--;
                }
                else{
                    ;
                }

            }
            else if(((PIND & 0x02) == 0x00) && ((PDsave_1&0x02) == 0x02)){//SW1 pressed
                _NOP();
                if(Atten < 11){
                    _NOP();
                    Atten++;
                }
                else{
                    ;
                }
            }
            else{//Both pressed or not pressed
                ;
            }

            if(Atten!=Atten_save){//When change occur
                _NOP();
                PORTD |= 0b00010000;//PD4 is 1
                PrintByte(LCDtext, "", Atten);
                PORTD &= 0b11101111;//PD4 is 0

                PORTD |= 0b00100000;//PD5 is 1
                LCD_MoveCursor(1,7);
                LCD_WriteString(LCDtext);
                PORTD &= 0b11011111;//PD5 is 0
            }
            else{
                ;
            }
            //_delay_us(500);
    };

    return(0);
}
