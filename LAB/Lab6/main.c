#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "LCDmodule.h"
#include <avr/interrupt.h>

int main(void)
{   DDRB |= 0b11111111; // set all pins of PORTB output
    DDRD |= 0b11111100; //input SW0 SW1, 456 output
    PORTD|= 0b00000011; //Pull-up resistor for inputs
    DDRC |= 0b11111100;

    PRR &= ~_BV(PRADC); //Enable power for ADC module

    ADMUX  = 0b00100001; // ADLAR=1, 8bit high
    /*              MUX[3:0] Single Ended Input
                        0000 ADC0
                        0001 ADC1
                        0010 ADC2
                        0011 ADC3
                        0100 ADC4
                        0101 ADC5
                        0110 ADC6
                        0111 ADC7
                        1000 Temperature sensor
                        1001 Reserved
                        1010 Reserved
                        1011 Reserved
                        1100 Reserved
                        1101 Reserved
                        1110 1.1V (VBG)
                        1111 0V (GND)                  */

    ADCSRA = 0b10000110; // ADEN ADSC ADATE ADIF ADIE ADPS2 ADPS1 ADPS0
    /*    ADPS[2:0] Division Factor
                000 2
                001 2
                010 4
                011 8
                100 16
                101 32
                110 64
                111 128                */
    //8M/64=125k <200k(max ADC frequency)

    ADCSRB = 0x00; //ADC B is disabled.
    DIDR0 |= 0b00000011; //Disabled buffer for ADC0 and ADC1


    PRR &= ~_BV(PRTIM0); //Enable Timer0
    PRR &= ~_BV(PRTIM1); //Enable Timer1

    TCNT0= 0x00; // Reset timer0 Counter
    TCNT1= 0x0000; // Reset timer1 Counter

    OCR0A = 0x00;// PWM duty cycle at OC0A=0

    OCR1A = 1000; // 1000*1us = 1ms

    TCCR0A = 0b10000011;//COM0A=10 (regular PWM output), WGM0=011(fast PWM mode);
    /*Mode WGM02 WGM01 WGM00 Timer/Counter Mode of Operation TOP Update of OCR0x at TOV Flag Set on(1)(2)
        0 0 0 0 Normal 0xFF Immediate MAX
        1 0 0 1 PWM, Phase Correct 0xFF TOP BOTTOM
        2 0 1 0 CTC OCRA Immediate MAX
        3 0 1 1 Fast PWM 0xFF BOTTOM MAX
        4 1 0 0 Reserved - - -
        5 1 0 1 PWM, Phase Correct OCRA TOP BOTTOM
        6 1 1 0 Reserved - - -
        7 1 1 1 Fast PWM OCRA BOTTOM TOP */

    TCCR0B = 0b00000001; // no prescaling for timer 0
        /*CA02 CA01 CS00 Description
            0    0    0  No clock source (Timer/Counter stopped).
        **  0    0    1  clkI/O/1 (No prescaling)
            0    1    0  clkI/O/8 (From prescaler)
            0    1    1  clkI/O/64 (From prescaler)
            1    0    0  clkI/O/256 (From prescaler)
            1    0    1  clkI/O/1024 (From prescaler)
            1    1    0  External clock source on T0 pin. Clock on falling edge.
            1    1    1  External clock source on T0 pin. Clock on rising edge.  */
    TCCR1A = 0x00; //no PWM
    TCCR1B = 0b00001010; // 1/8 clock scale counter clock of timer1 ; WGM1[3:0]=0100, clear timer on compare match
    TIMSK0 = 0x00; //no interrupt
    TIMSK1 = 0b00000010;//OCIE1A =1 , create interrupt when timer reach OCR1A


    LCD_Init();
    LCD_Clear();


    sei(); // Enable Interrupt
    while(1){;}//wait interrupt
    return 0;
}

ISR(TIMER1_COMPA_vect){
    static unsigned char LCDtext[16];
    static unsigned char Atten=1;
    static unsigned char PDsave = 0x00;
    static unsigned char PDsave_1 = 0x00;

    ADCSRA |= 0b01000000;//start ADC convert
    PORTD |= _BV(PORTD4); // Set ADC time marker 105us
    while((ADCSRA & 0b01000000) == 0b01000000){;} // wait ADC convert
    PORTD &= ~_BV(PORTD4); // Clear ADC time marker

    PDsave_1=PDsave;
    PDsave = PIND;

    if(((PIND & 0x01) == 0x00) && ((PDsave_1&0x01) == 0x01)){//SW0  pressed
        if(Atten > 1){
            Atten--;
        }
        else{
            ;
        }

    }
    else if(((PIND & 0x02) == 0x00) && ((PDsave_1&0x02) == 0x02)){//SW1 pressed
        if(Atten < 6){
            Atten++;
        }
        else{
            ;
        }
    }
    else{//Both pressed or not pressed
        ;//Do nothing
    }

    OCR0A=(ADCH>>(Atten-1)); // PWM duty-cycle adjustment


    PrintByte(LCDtext,"",ADCH);
    PORTD |= _BV(PORTD5); // Set LCD time marker 210us
    LCD_MoveCursor(1, 1); // Place LCD cursor
    LCD_WriteString(LCDtext); // Send LCDtext on screen
    PORTD &= ~_BV(PORTD5); // Clear LCD time marker
}   // end of ISR


