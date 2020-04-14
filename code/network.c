#include <mega32.h>
#include <alcd.h>
#include <stdio.h>
#include <delay.h>
#include <stdlib.h>

unsigned char message[20];
void keyboard(void);
unsigned char key;
unsigned int ind=0;
unsigned int dest=0;
unsigned int addr;

#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)
#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<DOR)
#define RX_BUFFER_SIZE 8

char rx_buffer[RX_BUFFER_SIZE];

unsigned char rx_wr_index=0,rx_rd_index=0;

unsigned char rx_counter=0;

// This flag is set on USART Receiver buffer overflow
bit rx_buffer_overflow;

// USART Receiver interrupt service routine
interrupt [USART_RXC] void usart_rx_isr(void)
{
char status,data;
unsigned int dest;
status=UCSRA;
data=UDR;
if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   {
   rx_buffer[rx_wr_index++]=data;
   if (rx_wr_index == RX_BUFFER_SIZE) rx_wr_index=0;
   if (++rx_counter == RX_BUFFER_SIZE)
      {
      rx_counter=0;
      rx_buffer_overflow=1;
      }
   }   
   
   if(ind == 0){                     
        dest = getchar()-'0' ;
        ind+=1;
        if(dest==addr){
            lcd_puts("REC from ");
            lcd_gotoxy(0,1); 
            } 
        else{  
            putchar(dest+'0'); 
            lcd_clear();
            lcd_puts("Sending To ");
            lcd_putchar(dest+'0');
        }   
   }
    
   
   message[ind] = getchar(); 
   if(message[ind]=='e'){
        ind = 0;
        lcd_gotoxy(0,0);
        if(dest!=addr) putchar('e');
    } 
               
    else{
        if(dest==addr)lcd_putchar(message[ind]); 
        else putchar(message[ind]);
        ind+=1; 
    } 
         
    
      
}

#ifndef _DEBUG_TERMINAL_IO_
// Get a character from the USART Receiver buffer
#define _ALTERNATE_GETCHAR_
#pragma used+
char getchar(void)
{
char data;
while (rx_counter==0);
data=rx_buffer[rx_rd_index++];
#if RX_BUFFER_SIZE != 256
if (rx_rd_index == RX_BUFFER_SIZE) rx_rd_index=0;
#endif
#asm("cli")
--rx_counter;
#asm("sei")
return data;
}
#pragma used-
#endif

void main(void)
{
// Input/Output Ports initialization
DDRA=(0<<DDA7) | (0<<DDA6) | (0<<DDA5) | (0<<DDA4) | (0<<DDA3) | (0<<DDA2) | (0<<DDA1) | (0<<DDA0);
PORTA=(0<<PORTA7) | (0<<PORTA6) | (0<<PORTA5) | (0<<PORTA4) | (0<<PORTA3) | (0<<PORTA2) | (0<<PORTA1) | (0<<PORTA0);

DDRB=(0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (0<<DDB2) | (1<<DDB1) | (1<<DDB0);
PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

DDRC=(0<<DDC7) | (1<<DDC6) | (1<<DDC5) | (1<<DDC4) | (1<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
PORTC=(0<<PORTC7) | (0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (0<<PORTC3) | (0<<PORTC2) | (0<<PORTC1) | (0<<PORTC0);

DDRD=(0<<DDD7) | (0<<DDD6) | (0<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);



// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud Rate: 4800
UCSRA=(0<<RXC) | (0<<TXC) | (0<<UDRE) | (0<<FE) | (0<<DOR) | (0<<UPE) | (0<<U2X) | (0<<MPCM);
UCSRB=(1<<RXCIE) | (0<<TXCIE) | (0<<UDRIE) | (1<<RXEN) | (1<<TXEN) | (0<<UCSZ2) | (0<<RXB8) | (0<<TXB8);
UCSRC=(1<<URSEL) | (0<<UMSEL) | (0<<UPM1) | (0<<UPM0) | (0<<USBS) | (1<<UCSZ1) | (1<<UCSZ0) | (0<<UCPOL);
UBRRH=0x00;
UBRRL=0x0C;


// Alphanumeric LCD initialization
// Connections are specified in the
// Project|Configure|C Compiler|Libraries|Alphanumeric LCD menu:
// RS - PORTA Bit 0
// RD - PORTA Bit 1
// EN - PORTA Bit 2
// D4 - PORTA Bit 4
// D5 - PORTA Bit 5
// D6 - PORTA Bit 6
// D7 - PORTA Bit 7
// Characters/line: 16
lcd_init(16);

// Global enable interrupts
#asm("sei")

addr = PINB.0+PINB.1*2;

while (1)
    {    
    while(1){
    key='';
    keyboard();
    if(key=='#')break;
    
    if(key!=''){
        if(message[0]=='\0')
            lcd_clear();
        lcd_putchar(key);
        message[ind]=key;
        ind+=1;
    }
    delay_ms(200);
    } 
    
    //assign destination
    lcd_gotoxy(0,1); 
    lcd_puts("dest:");
    key='';
    delay_ms(200);
    while(key == '')
        keyboard();
    dest=key - '0';
    if(dest == addr)
    {       
        lcd_clear();
        lcd_puts("It's me");
        delay_ms(200);
    }  
     else if(dest > 3)
    {       
        lcd_clear();
        lcd_puts("max address is 3");
        delay_ms(200);
    } 
    else {
    lcd_putchar(key);
    delay_ms(200);
    
    //sending message
    printf("%d%dMSG:%se",dest,addr,message);
    lcd_clear(); 
    lcd_puts("SENDING To ");
    lcd_putchar(dest+'0');
    delay_ms(20);
    lcd_gotoxy(0,1);
    lcd_puts(message);}
    
    //prepare situation for next round
    message[0] ='\0';
    ind = 0;
    
      }
}

void keyboard(void){
    //Row1
    PORTC.3=0;
    delay_ms(2);
    if(PINC.0==0)key='1';
    else if(PINC.1==0)key='2';
    else if(PINC.2==0)key='3';
    PORTC.3=1;
    //Row2
    PORTC.4=0;
    delay_ms(2);
    if(PINC.0==0)key='4';
    else if(PINC.1==0)key='5';
    else if(PINC.2==0)key='6';
    PORTC.4=1;
    //Row3
    PORTC.5=0;
    delay_ms(2);
    if(PINC.0==0)key='7';
    else if(PINC.1==0)key='8';
    else if(PINC.2==0)key='9';
    PORTC.5=1;
    //Row4
    PORTC.6=0;
    delay_ms(2);
    if(PINC.0==0)key='*';
    else if(PINC.1==0)key='0';
    else if(PINC.2==0)key='#';
    PORTC.6=1;
}