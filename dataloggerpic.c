#include <xc.h>
#include <pic16f628a.h>
#include <stdint.h>

#define _XTAL_FREQ   1843200
#define BAUDRATE              1200                      //bps
#define DataBitCount          8                         // no parity, no flow control
#define UART_RX               PORTB,RB0					// UART RX pin
#define UART_TX               PORTA,RA3					// UART TX pin
#define UART_RX_DIR			  TRISB,RB0					// UART RX pin direction register
#define UART_TX_DIR			  TRISA,RA3					// UART TX pin direction register
#define SCL     TRISAbits.TRISA1
#define SDA     TRISAbits.TRISA0
#define LED		RA2
#define SCL_IN  PORTA,RA1
#define SDA_IN  PORTA,RA0
#define BUTTON  PORTB,RB1
#define TMR0_2 (TMR0 & 1<<2)
#define LM75AD 0b10010000

unsigned char receive_serial_byte(void); 
#define SER_BAUD 		((_XTAL_FREQ / 4) / BAUDRATE)
#define SER_BIT			1			 	// Signal MODE - 1 = Normal 0 = Inverted (Use Inverted for direct 232)
//
// I/O Pins  
#define RxPin 			PORTB,RB0	// Input  
  
//

const char Help[20]=
{
'\r',//0
'\n',//1
'D',//2
'A',//3
'T',//4
'A',//5
' ',//6
'L',//7
'O',//8
'G',//9
'G',//10
'E',//11
'R',//12
' ',//13
'V',//14
'1',//15
'.',//16
'0',//17
'\r',
'\n'
};

const char Temp[16]=
{
',',//0
' ',//1
'T',//2
'e',//3
'm',//4
'p',//5
'e',//6
'r',//7
'a',//8
't',//9
'u',//10
'r',//11
'e',//12
' ',//13
'=',//14
' ',//15
};



#pragma config "FOSC=XT"
#pragma config "WDTE=OFF"
#pragma config "PWRTE=ON"
#pragma config "MCLRE=OFF"
#pragma config "CP=OFF"
#pragma config "CPD=OFF"
#pragma config "BOREN=OFF"
#pragma config "LVP=OFF"


   unsigned char count;
   unsigned char Log;
   unsigned char tempmsb;
   unsigned char templsb;
   unsigned char tempBCD1;
   unsigned char tempBCD2;
   unsigned char SampleBCD1;
   unsigned char SampleBCD2;
   unsigned char SampleBCD3;
   unsigned char SampleBCD4;
   unsigned char SampleBCD5;
   unsigned char loop;
   unsigned char LEDFLAG;	
   unsigned char SEC;
   unsigned char DATARDY;
   unsigned char Tick,Long;
   unsigned char DataValue;
   unsigned char TXENABLE;
   unsigned char i;
   unsigned char shift;
   unsigned char RX;
   unsigned char RxChr;
   unsigned int Sample;
   unsigned char Data;
   unsigned char Add_H;
   unsigned char Add_L;
   unsigned int temp;
   unsigned int y;
void i2c_dly(void)
{
_delay(20);
}


void i2c_start(void)
{
SDA = 1;
i2c_dly();
SCL = 1;
i2c_dly();
SDA = 0;
PORTA=0b11111100;
i2c_dly();
SCL = 0;
PORTA=0b11111100;
i2c_dly();
}





void i2c_stop(void)
{
  SDA = 0;             // i2c stop bit sequence
  PORTA=0b11111100;
  i2c_dly();
  SCL = 1;
  i2c_dly();
  SDA = 1;
  i2c_dly();
}


void sendchar(unsigned char c)
{

i=0;
DataValue = c;
TXENABLE=1;
__delay_ms(50);
		
}



bit i2c_tx(unsigned char d)
{
char x;
static bit b;
  for(x=8; x; x--) {
    if(d&0x80) SDA = 1;
    else SDA = 0;
	PORTA=0b11111100;
	i2c_dly();
    SCL = 1;
	i2c_dly();
    d <<= 1;
    SCL = 0;
	PORTA=0b11111100;
	i2c_dly();
  }
  SDA = 1;
  PORTA=0b11111100;
  i2c_dly();
  SCL = 1;
  PORTA=0b11111100;
  i2c_dly();
  b = SDA_IN;          // possible ACK bit
  i2c_dly();
  SCL = 0;
  PORTA=0b11111100;
  return b;
}

unsigned char i2c_rx(char ack)
{
unsigned char x, d=0;
 SDA = 1; 
  for(x=0; x<8; x++) {
    d <<= 1;
    do {
      SCL = 1;
    }
    while(SCL_IN==0);    // wait for any SCL clock stretching
    i2c_dly();
    if(SDA_IN) d |= 1;
    SCL = 0;
	PORTA=0b11111100;
  } 
  if(ack) SDA = 0;
  else SDA = 1;
  PORTA=0b11111100;
  i2c_dly();
  SCL = 1;
  PORTA=0b11111100;
  i2c_dly();
  i2c_dly();
  SCL = 0;
  PORTA=0b11111100;
  SDA = 1;
  PORTA=0b11111100;
  return (d) ;
}

unsigned char receive_serial_byte(void)
{
	T2CON = 0b00000001;
	PIE1bits.TMR2IE = 0;
	PR2=0xF0;
	TMR2IF = 0;
	TMR2ON = 1;
	unsigned char i;					// Bit Index
	i = 8;
								// 8 data bits to receive
	
	TMR2 =(256 - (SER_BAUD+19));		// load TMR2 value to offset ~center of RxBit
	while(TMR2>=20);					// wait for baud + offset
	while(i)							// receive 8 serial bits, LSB first
	{
		RxChr = (RxChr>>1);		    	// rotate right to store each bit
		if(RxPin == 1)				// save data bit
			 RxChr = RxChr | SER_BIT<<7;
		else RxChr = RxChr |!SER_BIT<<7;	
		
		i--;							// Next Bit
		TMR2 -= (SER_BAUD-17);				// load corrected baud value
		while(TMR2 & 1<<7);				// wait for baud
	}
	
	TMR2 -= SER_BAUD;					// wait for stop bit, ensure serial port is free
	while(TMR2 & 1<<7);
	TMR2ON = 0;
	return RxChr;
}





void InitSoftUART(void)		// Initialize UART pins to proper values
{
	UART_TX = 1;			// TX pin is high in idle state
	
}

void interrupt isr(void)
{
asm("nop");
asm("nop");
asm("nop");
asm("nop");
asm("nop");
asm("nop");
asm("nop");
asm("nop");
asm("nop");


	if(T0IF)
	{
		TMR0=234;
		T0IF = 0;
		GIE= 1;
		loop--;
	}
	if (loop==0)
	{
		loop=200;
		Long--;
		if (Long==0)
		{	
			SEC++;
			Long=6;
		}
	}

	

	if (TXENABLE==1)
	{
		if (i==0)
		{
			UART_TX = 0;
		}
		else if (i==9)
		{
			UART_TX = 1;
			i=0;
			TXENABLE=0;
		}	
		
		if(i>0 & i<9)
		{
			shift = i-1;
			if( ((DataValue>>shift) &0x1) == 0x1 )	
				{
				UART_TX = 1;
				}
				else      //if Bit is low
				{
				UART_TX = 0;
				}
		}
		i++;
	}
				
}

void WriteEEPROM(unsigned char Data)

{
i2c_start();
i2c_tx(0b10100010);
Add_H = Sample/256;
i2c_tx(Add_H);
Add_L =(Sample % 256);
i2c_tx(Add_L);
i2c_tx(tempmsb);
i2c_stop();
}

void ReadEEPROM()

{
i2c_start();
_delay(100);
i2c_tx(0b10100010);
_delay(100);
i2c_tx(Add_H);
_delay(100);
i2c_tx(Add_L);
_delay(100);
i2c_start();
i2c_tx(0b10100011);
_delay(100);
Data = i2c_rx(0);
i2c_stop();
}

void SampleBCD(void)
{
				
				SampleBCD1=(temp/10000);
				temp  = temp - (SampleBCD1 * 10000);
				SampleBCD2= temp /1000;
				temp  = temp - (SampleBCD2 * 1000);
				SampleBCD3= temp/100;
				temp  = temp - (SampleBCD3 * 100);
				SampleBCD4= temp/10;
				temp  = temp - (SampleBCD4 * 10);
				SampleBCD5=temp;
}				

void Tskesample(void)

{

				i2c_dly;
				i2c_start();
				i2c_tx(0b10010000);
				i2c_tx(0b00000000);
				i2c_start();
				i2c_tx(0b10010001);
				tempmsb=i2c_rx(1);
				templsb=i2c_rx(0);
				i2c_stop();
				__delay_ms(5);
				WriteEEPROM(tempmsb);
				tempBCD1=(tempmsb/10) + 48;
				tempBCD2=(tempmsb % 10) + 48;
				temp  = Sample;
				SampleBCD();
				
				DataValue = SampleBCD1 + 48;
				i=0;
				TXENABLE=1;
				__delay_ms(50);

				DataValue = SampleBCD2 + 48;
				i=0;
				TXENABLE=1;
				__delay_ms(50);

				DataValue = SampleBCD3 +48;
				i=0;
				TXENABLE=1;
				__delay_ms(50);

				DataValue = SampleBCD4 + 48;
				i=0;
				TXENABLE=1;
				__delay_ms(50);



				DataValue = SampleBCD5 + 48;
				i=0;
				TXENABLE=1;
				__delay_ms(50);


 				
				
				char x;			
				for(x=0; x<16; x++)
				 {
					sendchar(Temp[x]);				
				
				
				 }
			
				DataValue = tempBCD1;
				i=0;
				TXENABLE=1;
				__delay_ms(50);
 				DataValue = tempBCD2;
				i=0;
				TXENABLE=1;
				__delay_ms(50);
 				
				DataValue = '\r';
				i=0;
				TXENABLE=1;
				__delay_ms(50);

				DataValue = '\n';
				i=0;
				TXENABLE=1;
				__delay_ms(50);


}


void main()
{
	unsigned char ch = 0;
	i=0;
	SPEN=0;
	PORTA = 0b11100000;
	CMCON = 0b00000111;
	VRCON = 0b00000000;
	TRISA = 0b11110011;
	TRISB = 0b11111111;

	//INTEDG = 0; Interrupt on RB0 going H-L 	
	//INTE = 1; //Enable interrupt on RB0
	T0CS = 0;  // bit 5  TMR0 Clock Source Select bit...0 = Internal Clock (CLKO) 1 = Transition on T0CKI pin
	T0SE = 0;  // bit 4 TMR0 Source Edge Select bit 0 = low/high 1 = high/low
	PSA = 0;   // bit 3  Prescaler Assignment bit...0 = Prescaler is assigned to the Timer0
	PS2 = 0;   // bits 2-0  PS2:PS0: Prescaler Rate Select bits
	PS1 = 1;
	PS0 = 1;
	TMR0 = 232;             // preset for timer register


	T0IE = 1;
	PSA = 0;
    PEIE = 1;
	GIE = 1;


	TXENABLE = 0;
   	tempmsb=0;
   	templsb=0;
	count=0;
	Tick=200;
	Long= 6;
	LEDFLAG=0;
	SEC=0;
	Sample=0;
	loop=200;
	//T0IE=1;
	//GIE=1;			
	InitSoftUART();  	
   while(1)
   {
		PORTA,LED = 0;
		
		if(SEC==30)
		{
		
		SEC=0;
			if(Log==1)
			{
				Sample++;				
				PORTA,LED = 1;
				Tskesample();	
 			}					

		}
		
		if (BUTTON==0)
		{
			__delay_ms(50);
				
			if (BUTTON==0)
			{			
				if(Log==1)
					{
					Log=0;
					}
				if(Log==0)
					{
					Log=1;
					}
			}
		}

		if (UART_RX==0)
		{
		RX=receive_serial_byte();
		sendchar(RX);
			if(RX=='a')
			{
				SEC=0;	
				PORTA,LED = 1;
				Tskesample();	
 				
			}

			if(RX=='v')
			{
				Sample=0;			 				
			}

			if(RX=='c')
			{
					DataValue = SampleBCD1 + 48;
					i=0;
					TXENABLE=1;
					__delay_ms(10);

					DataValue = SampleBCD2 + 48 ;
					i=0;
					TXENABLE=1;
					__delay_ms(10);

					DataValue = SampleBCD3 + 48 ;
					i=0;
					TXENABLE=1;
					__delay_ms(10);

					DataValue = SampleBCD4 + 48 ;
					i=0;
					TXENABLE=1;
					__delay_ms(10);



					DataValue = SampleBCD5 +48;
					i=0;
					TXENABLE=1;
					__delay_ms(10);

			}

			if(RX=='s')
			{
				SEC=0;	
				Log=1;	 				
			}

			if(RX=='t')
			{
				SEC=0;	
				Log=0;	 				
			}

			if(RX=='b')
			{
				char x;			
				for(x=0; x<20; x++)
				 {
					sendchar(Help[x]);				
				
				
				 }
			}

			if(RX=='r')
			{

				Log = 0;			
				for(y=1; y<(Sample+1); y++)
				 	{
					Add_H = y/256;
					Add_L = y -(Add_H*256);		
					ReadEEPROM();
					tempmsb = Data;
					tempBCD1=(tempmsb/10) + 48;
					tempBCD2=(tempmsb % 10) + 48;
				
					temp=y;
					SampleBCD();
				
					DataValue = '\r';
					i=0;
					TXENABLE=1;
					__delay_ms(10);

					DataValue = '\n';
					i=0;
					TXENABLE=1;
					__delay_ms(10);
					
					DataValue = SampleBCD1 + 48;
					i=0;
					TXENABLE=1;
					__delay_ms(10);

					DataValue = SampleBCD2 + 48 ;
					i=0;
					TXENABLE=1;
					__delay_ms(10);

					DataValue = SampleBCD3 + 48 ;
					i=0;
					TXENABLE=1;
					__delay_ms(10);

					DataValue = SampleBCD4 + 48 ;
					i=0;
					TXENABLE=1;
					__delay_ms(10);



					DataValue = SampleBCD5 +48;
					i=0;
					TXENABLE=1;
					__delay_ms(10);


					DataValue = ',';
					i=0;
					TXENABLE=1;
					__delay_ms(10);

				
				DataValue = tempBCD1;
				i=0;
				TXENABLE=1;
				__delay_ms(10);
 				DataValue = tempBCD2;
				i=0;
				TXENABLE=1;
				__delay_ms(10);
 				
				}		
				
				
				DataValue = '\r';
				i=0;
				TXENABLE=1;
				__delay_ms(10);

				DataValue = '\n';
				i=0;
				TXENABLE=1;
				__delay_ms(10);

				DataValue = 'E';
				i=0;
				TXENABLE=1;
				__delay_ms(10);
				
				
			}
		}

	}
}

	
			
