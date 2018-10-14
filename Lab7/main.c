#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "LCDmodule.h"
#include <avr/interrupt.h>
#define  LCDbufferSize    32

// Global variables for data transfer to timer-2 ISR:
unsigned char  LCDbuffer[LCDbufferSize];
unsigned char  Nbyte = 0;
unsigned char Atten = 1;
unsigned char LcdEn;

void DeQueueLCDbuffer(void){
  static unsigned char  *pLCDout = LCDbuffer;
  static unsigned char  Bcount = LCDbufferSize;
  unsigned char  ByteOut;

  if (Nbyte > 0)
// First send the 4 most significant bits of the LCD data:
  { ByteOut = *pLCDout >> 4;
// Set the register select bit, RS=1, when sending display data to LCD:
    if ( (*pLCDout & 0x80) == 0 )  ByteOut |= 0x10;
// Write 4 MSBs:
    _LCDport = ByteOut;
    _LCDport = 0x40 | ByteOut;  // Set Enable
    _LCDport = ByteOut;         // Clear Enable
    ByteOut = (ByteOut & 0xF0) | (*pLCDout & 0x0F);
    _LCDport = ByteOut;
    _LCDport = 0x40 | ByteOut;  // Set Enable
    _LCDport = ByteOut;         // Clear Enable
    _LCDport = 0x00;  // clear port output
// Update the LCDbuffer status:
    Bcount --;        // decrement buffer position counter
    if (Bcount == 0)  // check if reached end of buffer array
    { pLCDout = LCDbuffer;     // go back to the first element
      Bcount = LCDbufferSize;  // reload buffer position counter
    }
    else
      pLCDout ++;  // increment dequeue buffer pointer
    Nbyte --;      // decrement number of stored bytes
  }
  return;
}

void EnQueueLCDbuffer(unsigned char ByteIn){
    static unsigned char *pLCDin=LCDbuffer;
    static unsigned char Bcount=LCDbufferSize;
    if(Nbyte<32)
    {
        *pLCDin=ByteIn;
        Bcount--;
        if(Bcount==0)
        {
            pLCDin=LCDbuffer;
            Bcount=LCDbufferSize;
        }
        else
        {
            pLCDin++;
        }
        Nbyte++;
    }
    return;
}

ISR(TIMER1_COMPA_vect) //ADC convert 1ms interrupt time
{
    PORTD |= _BV(PORTD4);
    //static unsigned char PulseCount = 0;
    static char LCDtext[16];
    static char *pText;
    ADCSRA |= 0x40;

    while(ADCSRA & 0x40);

    //OCR0A = ADCH;

    if(LcdEn==0){
    PrintByte(LCDtext,"",ADCH);

    EnQueueLCDbuffer(0x80);

    pText = LCDtext;

        while (*pText != 0x00)
        {EnQueueLCDbuffer(*pText);
            pText ++;
        };
        TIMSK2 = 0x02;
    }

    if(Atten%2 == 0)
        OCR0A = (ADCH>>(Atten>>1)) + (ADCH>>((Atten>>1) + 1));
    else
        OCR0A = (ADCH>>((Atten - 1)>>1));
    PORTD &= ~_BV(PORTD4);
}

ISR(TIMER2_COMPA_vect){//LCD yazdırma 50us
    PORTD |= _BV(PORTD5);
    if(LcdEn == 0){
        DeQueueLCDbuffer();
        if(Nbyte == 0)
            TIMSK2 = 0x00; // block interrupt
        else{
            TCNT2 =0x00;
            TIFR2 =0x02;
        }
    }
    PORTD &= ~_BV(PORTD5);
}

int main(void)
{
    static unsigned char PDsave = 0x00;
    static unsigned char PDsave_old = 0x00;
    static char *pText;
    static char LCDtext[16];
    DDRB |= 0xFF;
    PORTD|= 0b00000011; //Pull-up resistor for inputs
    DDRD |= 0xFC;//(1111 1100)
    LCD_Init();
    LCD_Clear();
    //char LCDtext[16];
    ADMUX = 0x20;
    ADCSRA = 0x86;
    OCR0A = 0x00;
    TCCR0A = 0x83;//1000 0000;
    TCCR0B = 0x01;//0000 0100 CS0[2:0]=100 1/256 clock freq
    TIMSK0 = 0x00;
    OCR1A = 1000;//to obtain an interrupt once every 1 ms
    TCCR1A = 0x00;
    TCCR1B = 0x0A;//0000 1010 CS1[2:0]=010 1/8 clock freq
    TIMSK1 = 0x02;//enables the interrupt source
    TCNT0 = 0x00;
    TCNT1 = 0x00;

    OCR2A = 50;
    TCCR2A = 0x02;
    TCCR2B = 0x02;//0000 0100 CS0[2:0]=100 1/8 clock freq
    TIMSK2 = 0x02;

    PrintByte(LCDtext,"", Atten);
    LCD_MoveCursor(2, 1);
    LCD_WriteString(LCDtext);

    sei();
    while(1){

        PDsave_old=PDsave;
        PDsave = PIND;

        if(((PIND & 0x01) == 0x00) && ((PDsave_old&0x01) == 0x01)){
            if(Atten > 1){
                Atten--;

                LcdEn = 1;
                PrintByte(LCDtext,"", Atten);
                //LCD_MoveCursor(2, 1);
                EnQueueLCDbuffer(0xC0);

                //LCD_WriteString(LCDtext);
                pText = LCDtext;

                while (*pText != 0x00)
                {EnQueueLCDbuffer(*pText);
                    pText ++;
                };
                LcdEn = 0;
                TIMSK2 = 0x02;// can create interrupt

                        //sprintf(LCDtext,"%3d", Atten);
            }
        }
        else if(((PIND & 0x02) == 0x00) && ((PDsave_old&0x02) == 0x02)){
            if(Atten < 11){
                Atten++;

                LcdEn = 1;
                PrintByte(LCDtext,"", Atten);
                //LCD_MoveCursor(2, 1);
                EnQueueLCDbuffer(0xC0);

                //LCD_WriteString(LCDtext);
                pText = LCDtext;

                while (*pText != 0x00)
                {EnQueueLCDbuffer(*pText);
                    pText ++;
                };
                LcdEn = 0;
                TIMSK2 = 0x02;// can create interrupt

                //sprintf(LCDtext,"%3d", Atten);
            }
        }


    };
    return 0;
}
