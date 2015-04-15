/*
 *  styrmodul_test1.c
 *	Styrmodul
 *  Author: adnbe196, mansk700, nikag669
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <math.h>

//#include <avr/pgmspace.h>
#define F_CPU 20000000UL

// GLOBAL VARIABLES

int LCD_Counter;

// Setup data direction registers @ ports for out/inputs.
void Styr_InitPortDirections(void)
{
	DDRA = 1<<DDA0 | 1<<DDA1 | 1<<DDA2 | 1<<DDA3 | 1<<DDA4 | 1<<DDA5 | 1<<DDA6 | 1<<DDA7;
	DDRB = 1<<DDB0 | 1<<DDB1 | 1<<DDB2 | 1<<DDB3 | 1<<DDB4 | 1<<DDB5 | 1<<DDB7;
	DDRC = 1<<DDC0;
	DDRD = 1<<DDD0 | 1<<DDD1 | 1<<DDD2 | 1<<DDD3 | 1<<DDD4 | 1<<DDD5 | 1<<DDD6 | 1<<DDD7;
} 

// Setups port values, more specifically puts SS on high.
void Styr_InitPortValues(void)
{
	PORTB = 1<<PORTB3 | 1<<PORTB4 | 1<<PORTC0;
}

// Configures device as spi master.
void SPI_MasterInit(void)
{
	SPSR = 0<<SPI2X;
	SPCR = 0<<SPIE | 1<<SPE | 1<<DORD | 1<<MSTR | 0<<CPOL | 0<<CPHA | 1<<SPR1 | 1<<SPR0;
	// SPIE: SPI interrupt enable. Set to 1 to allow interrupts
	// SPE: SPI Enable. Set to 1 to allow SPI communication
	// DORD: Data order. Set to 1 to transmit LSB first, MSB last.
	// MSTR: Master select. Set to 1 to set master.
	// CPOL and CPHA: set to 0 to sample on rising edge and setup on falling edge.
	// SPI2X, SPR1, SPR0, set to 0,1,1 to scale clock with f_osc/128.
	// Bus config similar to comm and sensor module, though set 0<<MSTR
	
}

// Initiates communication with other modules.
unsigned char SPI_MasterTransmit(unsigned char cData, char target)
{
	if (target == 'k') // K as in kommunikation
	{
		PORTB &= ~(1<<PORTB4);
	}
	else if (target == 's')	// S as in sensor
	{
		PORTB &= ~(1<<PORTB3);
	}
	else if (target == 'g') // G as in gyro
	{
		PORTC &= ~(1<<PORTC0);
	}
	// Load data into SPI data register.
	SPDR = cData; 
	
	// Wait until transmission completes.
	while(!(SPSR & (1<<SPIF)));
	
	// Reset SS.
	PORTB |= 1<<PORTB3 | 1<<PORTB4;
	PORTC |= 1<<PORTC0;
	
	return SPDR;
}

//ISR(SPI_STC_vect)
//{
	//PORTB |= 1<<PORTB3 | 1<<PORTB4 | 1<<PORTC0;
//}

//----------------------------PWM------------------------------------

void PWM_Init(void)
{
	// TODO:::::s�tt riktning p� databitar
	
	// Configuration of the PWM setting for OC2A and OC2B
	// TCCn1:0 = 3 equals phase correct PWM mode
	// WGM22:0 = 5 equals phase correct PWM mode with compare from OCRn
	// CS22:0 = 3 sets the division factor to 32
	TCCR2A = 0<<COM2A1 | 1<<COM2A0 | 0<<COM2B1 | 1<<COM2B0 | 0<<WGM21 | 1<<WGM20;
	TCCR2B = 1<<WGM22 | 0<<CS22 | 1<<CS21 | 1<<CS20;
	//TIMSK2 = 1<<OCIE2B | 1<<OCIE2A;
	
	// Set the compare values for the PWM, 0 == 0% and 255 for 100%
	OCR2A = 0;
	OCR2B = 0;
}

void PWM_SetSpeedRight(int speed)
{
	if (speed >= 0 && speed <= 255)
	{
		OCR2B = speed;
	}
}

void PWM_SetSpeedLeft(int speed)
{
	if (speed >= 0 && speed <= 255)
	{
		OCR2A = speed;
	}
}

void PWM_SetDirLeft(int dir)
{
	if (dir == 0)
	{
		PORTD &= ~(1 << PORTD0);
	} 
	//&= ~(1 << PORTD0)
	else if (dir == 1)
	{
		PORTD |= 1 << PORTD0;
	}
}

void PWM_SetDirRight(int dir)
{
	if (dir == 0)
	{
		PORTD &= ~(1 << PORTD1);
	}
	else if (dir == 1)
	{
		PORTD |= 1 << PORTD1;
	}
}

void PWM_Test(void)
{
	for (int i=0; i < 300; i++)
	{
		_delay_ms(250);
	}
	PWM_SetSpeedLeft(50);
	//PWM_SetSpeedRight(0);
	for (int i=0; i < 300; i++)
	{
		_delay_ms(250);
	}
	PWM_SetDirLeft(1);
	//PWM_SetDirRight(0);
	for (int i=0; i < 300; i++)
	{
		_delay_ms(250);
	}
	PWM_SetSpeedLeft(0);
	//PWM_SetSpeedRight(150);
	for (int i=0; i < 300; i++)
	{
		_delay_ms(250);
	}
	//PWM_SetDirRight(1);
	for (int i=0; i < 300; i++)
	{
		_delay_ms(250);
	}
}

// Input argument dir = 0 for down, dir = 1 for up.
void SERVO_SetVertical(uint8_t dir)
{
	if (dir == 0)
	{
		PORTD &= ~(1 << PORTD2);
	}
	else if (dir == 1)
	{
		PORTD |= (1 << PORTD2);
	}
}

// Input argument dir = 0 for release, dir = 1 for grip.
void SERVO_SetGrip(uint8_t dir)
{
	if (dir == 0)
	{
		PORTD &= ~(1 << PORTD3);
	}
	else if (dir == 1)
	{
		PORTD |= (1 << PORTD3);
	}
}
//----------------------------PWM------------------------------------

int LCD_Busy()
{
	DDRA = 0; // Set PORTA as input
	PORTB |= 1 << 1; // Read busy flag
	PORTB &= ~(1 << 0); // Clear bit 0 in PORTB to 0 (Set register select to instruction "RS=0")
	PORTB |= (1 << 2); // Activates the LCD (enable pin on LCD)
	_delay_us(2);
	char instr = PINA;
	PORTB &= ~(1 << 2); // Clear bit 2 in PORTB
	PORTB &= ~(1 << 1); // Clear bit 1 in PORTB
	DDRA = 0xff; // Set PORTA as output
	return instr >> 7; // MSB is the busy flag
}

void LCD_SendCommand(char cmd) {
	while (LCD_Busy())
	{
		_delay_ms(1);
	}
	
	PORTB &= ~(1 << 0); // Clear bit 0 in PORTB to 0 (Set Register Select to instruction "RS=0")
	PORTA = cmd;
	PORTB |= 1 << 2; // Set R/W to 1.
	
	if (cmd == 0b01 || cmd == 0b10) // If clear or return home instruction
	{
		_delay_ms(1.5);				    // Then execution time is longer
	}
	else
	{
		_delay_us(50);
	}

	PORTB &= ~(1 << 2);
}

// Sets row on the LCD. Do not use this yourself, rather use LCD_SetPosition instead.
void LCD_SetRow(int row)
{
	while(LCD_Busy())
	{
		_delay_ms(1);
	}
	
	PORTB &= ~(1 << 0); // Clear RS and
	PORTB &= ~(1 << 1); // clear R/W bits so that the following commands can be run
	
	if (row == 1)
	{
		LCD_SendCommand(0b10000000); // Set the cursor on the first row, first char
	}
	else if (row == 2)
	{
		LCD_SendCommand(0b11000000); // Set the cursor on the second row, first char
	}
}

// Sets position for cursor on LCD. Argument should be a number in the range of 0-31.
void LCD_SetPosition(uint8_t pos)
{
	LCD_Counter =(int) pos - 1;
	while(LCD_Busy())
	{
		_delay_ms(1);
	}
	PORTB &= ~(1 << 0); // Clear RS and
	PORTB &= ~(1 << 1); // clear R/W bits so that the following commands can be run
	
	if (pos < 16)
	{
		LCD_SendCommand(128+pos);
	}
	else if (pos < 32)
	{
		LCD_SendCommand(128+64-16+pos);
	}
	else LCD_SendCommand(0b10000000);
	
}
void LCD_SendCharacter(char symbol)
{
	LCD_Counter++;
	
	if(LCD_Counter == 16)
	{
		LCD_SetRow(2);
	}
	else if(LCD_Counter == 32)
	{
		LCD_Counter = 0;
		LCD_SetRow(1);
	}
	
	while(LCD_Busy())
	{
		_delay_ms(1);
	}
	PORTB |= (1 << 0); // Set RS
	PORTB &= ~(1 << 1); // Clear R/W
	
	//uint8_t tempNum = (int)symbol;
	//PORTA = tempNum;
	
	// If the following doesn't work, delete it and uncomment the two lines above.
	PORTA = (int)symbol;
	
	PORTB |= 1 << 2; // Set Enable
	_delay_us(50); // 50us is the controller execution time of the LCD.
	PORTB &= ~(1 << 2); // Pull Enable.
}

void LCD_SendString(char *text)
{
	while(*text)
	{
		LCD_SendCharacter(*text++);
	}
}
void LCD_WelcomeScreen(void)
{
	LCD_SendString("    ResQ.Pl heh");
	LCD_SetRow(2);
	LCD_SendString("  Master Race  ");
}

// Initiatazion of the LCD, according to Initializing Flowchart(Condition fosc=270KHz) in the data sheet.
void LCD_Init()
{
	LCD_Counter = 0;
	_delay_ms(30);
	// Configure the LCD for 8 bits, 2 lines, 5x8 pixlex (dots) send instruction 00 0011 1000
	LCD_SendCommand(0b00111000);
	_delay_us(39);

	// Display, cursor and blinking off, instruction 00 0000 1000
	LCD_SendCommand(0b00001000);
	_delay_us(39);

	// Clear display, instruction 00 0000 0001
	LCD_SendCommand(0b00000001);
	_delay_ms(1.53);

	// Cursor moving direction: left-to-right, do not shift he display (shift disabled), instruction 00 0000 0110
	LCD_SendCommand(0b00000110);

	// Display on, cursor ON, blinking on, instruction 00 0000 1110
	LCD_SendCommand(0b00001110);
}

int main(void)
{
	char SPDRrec_ = '0';
	sei();	// Enable global interrupts
	sleep_enable();	// Enable sleep instruction
	Styr_InitPortDirections();	// Initiate Port directions for the styrmodul.
	Styr_InitPortValues();	// Initiate Port Values for the styrmodul.
	SPI_MasterInit();	// Initiate the styrmodul as the SPI master.
	LCD_Init(); // Initiate the LCD.
	PWM_Init(); // Initiate PWM for mot�r
	LCD_WelcomeScreen(); // Welcomes the user with a nice message ;-)
	_delay_ms(250);
	
 	//SPDRrec_ = SPI_MasterTransmit(0x01, 'k');
	//

	LCD_SetRow(1);
	LCD_SendCharacter('7');
	SPDRrec_ = SPI_MasterTransmit(1, 'k');
	LCD_SendCharacter(SPDRrec_);
	SPDRrec_ = SPI_MasterTransmit(2, 'k');
	LCD_SendCharacter(SPDRrec_);
	
	
	
	//LCD_SetPosition(31);
	//_delay_ms(15000);
	//LCD_SendCharacter('T');
	//_delay_ms(15000);
	
	//LCD_SendCharacter(SPDRrec_);
	
	//----------------------------------TEST--PWM------------------------------------------------------
	
	//LEFTE
	//PWM_SetDirLeft(1);
	//PWM_SetSpeedLeft(50);
	//for (int i=0; i < 300; i++)
	//{
		//_delay_ms(250);
	//}
	//
	//PWM_SetSpeedLeft(200);
	//for (int i=0; i < 300; i++)
	//{
		//_delay_ms(250);
	//}
	//PWM_SetDirLeft(0);
	//
	//for (int i=0; i < 500; i++)
	//{
		//_delay_ms(250);
	//}
	//
	////PWM_SetSpeedLeft(0);
	//
	////Richty
	//PWM_SetDirRight(1);
	//PWM_SetSpeedRight(50);
	//for (int i=0; i < 300; i++)
	//{
		//_delay_ms(250);
	//}
	//
	//PWM_SetSpeedRight(200);
	//for (int i=0; i < 300; i++)
	//{
		//_delay_ms(250);
	//}
	//PWM_SetDirRight(0);
	//
	//for (int i=0; i < 500; i++)
	//{
		//_delay_ms(250);
	//}
	
	//PWM_SetSpeedRight(100);
	//PWM_SetSpeedLeft(100);
	
	//----------------------------------TEST--PWM------------------------------------------------------
	
	//activate ADC in gyro, if it was not active already ("initieringen")
	//char dataH, dataL;
	//SPDRrec_ = SPI_MasterTransmit(0b10010100,'g');
	//SPDRrec_ = SPI_MasterTransmit(0,'g');
	//SPDRrec_ = SPI_MasterTransmit(0,'g');
	//if (!(SPDR &= 0b10000000)) { // if bit 7 is zero, the instruction is accepted
	//	LCD_SendString("instr1 accepted");
	//	while (!(SPDR &= 0b00100000)) { // wait for EOC bit (bit 5) to be set - means AD-conversion is complete
	//		_delay_us(5);  // instead of while loop can just wait for > 115us
	//	}
	//}
	//else LCD_SendString("wrong instr1");
	
	//PWM_SetDirLeft(1);
	//PWM_SetSpeedRight(200);
	
	while(1)
    {
		SPDRrec_ = SPI_MasterTransmit(0,'k');
		LCD_SendCharacter(SPDRrec_);
		_delay_ms(5000);
		PWM_Test();
		
		
		////---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		//PORTD &= ~(1 << PORTD6);
		//PORTD &= ~(1 << PORTD7);
		//_delay_ms(8);
		//PORTD |= 1 << PORTD6;
		//PORTD |= 1 << PORTD7;
		//_delay_ms(5);		
		////-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		
				
		//// start the conversion (notera att samma instruktion skickas vid aktivering av ADC:en i gyro)
		//SPDRrec_ = SPI_MasterTransmit(0b10010100,'g'); //ss to 0, enabling the sensor
		//SPDRrec_ = SPI_MasterTransmit(0,'g');
		////SPDRrec_ = SPI_MasterTransmit(0,'g');
		//if (!(SPDR &= 0b10000000)) { // if bit 7 is zero, the instruction is accepted
			//LCD_SendString("instr2 accepted");
			//}
		//else LCD_SendString("wrong instr2");
		
		//// polling and result obtaining
		//SPI_MasterTransmit(0b10000000, 'g'); // send SPI ADCR instruction
		//// mb need to check "instr accepted" bit and "and conversion done" bit before continuing.....
		//_delay_us(120); //but do this for now
		//dataH = SPI_MasterTransmit(0x00, 'g'); // get the sensor response high byte
		//dataL = SPI_MasterTransmit(0x00, 'g'); // get the sensor response low byte
		//PORTC |= 1<<PORTC0; //set ss to 1; disabling the sensor
		//// unsigned int result = makeWord((dataH & 0b00001111), (dataL>>1));
		
		
		
		//
		//SPDRrec_ = SPI_MasterTransmit(0,'k');
		//LCD_SendCharacter(SPDRrec_);
		//_delay_ms(15000);
		//SPDRrec_ = SPI_MasterTransmit(0,'k');
		//LCD_SendCharacter(SPDRrec_);
		//_delay_ms(15000);
		//
		//SPDRrec_ = SPI_MasterTransmit(3,'k');
		//LCD_SendCharacter(SPDRrec_);
		//_delay_ms(15000);
		////LCD_SetRow(1);
		//SPDRrec_ = SPI_MasterTransmit(4,'k');
		//LCD_SendCharacter(SPDRrec_);
		//_delay_ms(15000);
		//SPDRrec_ = SPI_MasterTransmit(5,'k');
		//LCD_SendCharacter(SPDRrec_);
		//_delay_ms(15000);
		//LCD_SetRow(1);
	}
}