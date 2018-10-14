#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "LCDmodule.h"
#include <avr/interrupt.h>
#define LCDbufferSize    32
#define bufferSize 32

// Global variables for data transfer to timer-2 ISR:
unsigned char  LCDbuffer[LCDbufferSize];
unsigned char  buffer[bufferSize];
unsigned char  Nbyte = 0;
unsigned char Ubyte = 0;
unsigned char Atten = 1;
unsigned char LcdEn;

void DeQueueLCDbuffer(void){
  static unsigned char  *pLCDout = LCDbuffer;
  static unsigned char  Bcount = LCDbufferSize;
  unsigned char  ByteOut;
    PORTD |= _BV(PORTD4);
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
  PORTD &= ~_BV(PORTD4);
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

void EnQueueSerialBuffer(unsigned char ByteIn){
    static unsigned char *pin=buffer;
    static unsigned char Bcount=bufferSize;
    if(Ubyte<32)
    {
        *pin=ByteIn;
        Bcount--;
        if(Bcount==0)
        {
            pin=buffer;
            Bcount=bufferSize;
        }
        else
        {
            pin++;
        }
        Ubyte++;
    }
    return;
}

void DeQueueSeraialbuffer(void){
  static unsigned char  *pout = buffer;
  static unsigned char  Bcount = bufferSize;

  if (Ubyte > 0)
  { UDR0 = *pout;
// Set the register select bit, RS=1, when sending display data to LCD:
    Bcount --;        // decrement buffer position counter
    if (Bcount == 0)  // check if reached end of buffer array
    { pout = buffer;     // go back to the first element
      Bcount = bufferSize;  // reload buffer position counter
    }
    else
      pout ++;  // increment dequeue buffer pointer
    Ubyte --;      // decrement number of stored bytes
  }
  return;
}

ISR(TIMER1_COMPA_vect) //ADC convert 1ms interrupt time
{

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

}

ISR(TIMER2_COMPA_vect){ //LCD print
    if(LcdEn == 0){
        DeQueueLCDbuffer();
        if(Nbyte == 0)
            TIMSK2 = 0x00; // close timer2 when finish
        else{
            TCNT2 =0x00;
            TIFR2 =0x02;
        }
    }
}

ISR(USART_RX_vect){
    static char *pText;
    static char LCDtext[16];
    static char received;
    received = UDR0;

    if((received & 0xDF) == 0x44){
            if(Atten > 1){
                Atten--;
            }
    }
    else if((received & 0xDF) == 0x55){
            if(Atten < 11){
                Atten++;
            }
    }
    if((received & 0xDF) == 0x55 || (received & 0xDF) == 0x44){
        LcdEn = 1;
        PrintByte(LCDtext,"", Atten);
        //LCD_MoveCursor(2, 1);
        EnQueueLCDbuffer(0xC0);
        //LCD_WriteString(LCDtext);
        pText = LCDtext;
        while (*pText != 0x00){
            EnQueueLCDbuffer(*pText);
            pText ++;
    };
    LcdEn = 0;
    TIMSK2 = 0x02;

    }
    UCSR0B = 0x78;//01111000
    EnQueueSerialBuffer(0x0D);
    pText = LCDtext;
        while (*pText != 0x00){
            EnQueueSerialBuffer(*pText);
            pText ++;
    };
    UCSR0B = 0x98;
}

ISR(USART_UDRE_vect){ //uart gönder
    PORTD |= _BV(PORTD5);
    DeQueueSeraialbuffer();
    PORTD &= ~_BV(PORTD5);
}
int main(void){
    //static unsigned char PDsave = 0x00;
    //static unsigned char PDsave_1 = 0x00;
    DDRB |= 0xFF;
    DDRC |= 0xFF; // LCD at PORT-C
    DDRD |= 0xFC;//(1111 1100)
    LCD_Init();
    LCD_Clear();
    char LCDtext[16];
    PrintByte(LCDtext,"", Atten);
    LCD_MoveCursor(2, 1);
    LCD_WriteString(LCDtext);

    ADMUX = 0x20;
    ADCSRA = 0x86;
    OCR0A = 0x00;
    TCCR0A = 0x83;//1000 0000;
    TCCR0B = 0x01;//0000 0100 CS0[2:0]=100 1/256 clock freq
    TIMSK0 = 0x00;
    OCR1A = 10000;//to obtain an interrupt once every 10 ms
    TCCR1A = 0x00;
    TCCR1B = 0x0A;//0000 1010 CS1[2:0]=010 1/8 clock freq
    TIMSK1 = 0x02;//enables the interrupt source
    TCNT0 = 0x00;
    TCNT1 = 0x00;

    TCCR2A = 0x02;
    TCCR2B = 0x02;//0000 0100 CS0[2:0]=100 1/8 clock freq
    //TIMSK2 = 0x02;

    //initialization of USART
    UBRR0 = 51; // set boundrate to 9600
    UCSR0A = 0x00;
    UCSR0B = 0x98;//10011000;
    UCSR0C = 0x06;//00000110;  UCSZ0=011,8 bit

    sei();
    while(1){};
    return 0;
}
