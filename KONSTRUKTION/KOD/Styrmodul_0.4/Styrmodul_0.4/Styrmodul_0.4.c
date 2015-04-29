﻿/*
 * pwmpls.c
 *
 * Created: 4/13/2015 12:39:10 PM
 *  Author: robop806
 */ 

#define F_CPU 20000000UL

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <math.h>
#include <string.h>
#include "Styrmodul_LCD.c"
#include "Styrmodul_Servo.c"
//#include <avr/pgmspace.h>

#define AUTONOM_MODE (((PIND) & (1<<PIND3)) >> 3)
#define MANUAL_MODE (!AUTONOM_MODE)


// GLOBAL VARIABLES --------------------------------------------------------------------
uint8_t arrSpeed[] = {0,0,1,1,0}; //speedLeft, SpeedRight, DirLeft, DirRight, grip
int8_t arrSensor[] = {-1,0,0,0}; //sensor 0-3
// One overflow corresponds to 13.1 ms if the F_CPU is 20 MHz and the defined prescaler.
// This timer will reset every time it reaches 10.
uint8_t TIMER_overflows;
// This timer will NOT reset every time it reaches 10.
int16_t TIMER_wheel;
// The following overflow storage corresponds to 131ms. This timer will never reset.
uint8_t TIMER_overflows_deci;
// This variable is used to store robots angle in degrees
int rotation_angle = 0;
//KOntrollerar att flaggor sätts korekkt vi Manuell / Autonom loop
int Manual_Flag = 0;

// --------------------------------------------------------------------------------------

// Setup data direction registers @ ports for out/inputs.
void Styr_InitPortDirections(void)
{
	DDRA = 1<<DDA0 | 1<<DDA1 | 1<<DDA2 | 1<<DDA3 | 1<<DDA4 | 1<<DDA5 | 1<<DDA6 | 1<<DDA7;
	DDRB = 1<<DDB0 | 1<<DDB1 | 1<<DDB2 | 1<<DDB3 | 1<<DDB4 | 1<<DDB5 | 1<<DDB7;
	DDRC = 1<<DDC0;
	DDRD = 1<<DDD0 | 1<<DDD1 | 0<<DDD2 | 0<<DDD3 | 1<<DDD4 | 1<<DDD5 | 1<<DDD6 | 1<<DDD7;
	DDRD &= ~(1 << DDD3);
	//D2 ingеng fцr reflexsensor
	// 0 = input
	// 1 = output
}

// Setups port values, more specifically puts SS on high.
void Styr_InitPortValues(void)
{
	PORTB |= 1<<PORTB3 | 1<<PORTB4;
	PORTC |= 1<<PORTC0;
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
unsigned char SPI_MasterTransmit(uint8_t cData, char target)
{
	if (target == 'k') // K as in kommunikation
	{
		PORTB &= ~(1<<PORTB4);
	}
	else if (target == 's')	// S as in sensor
	{
		PORTB &= ~(1<<PORTB3);
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



//----------------------------PWM------------------------------------

void PWM_Init(void)
{	
	// Configuration of the PWM setting for OC2A and OC2B
	// COM2n1:0 = 2 equals clear OC2A/B on compare match, and set at bottom
	// WGM22:0 = 3 equals fast PWM mode
	// CS22:0 = 4 sets the division factor to 64
	TCCR2A = 1<<COM2A1 | 0<<COM2A0 | 1<<COM2B1 | 0<<COM2B0 | 1<<WGM21 | 1<<WGM20;
	TCCR2B = 0<<WGM22 | 1<<CS22 | 0<<CS21 | 0<<CS20;	
	
	// Set the compare values for the PWM, 0 == 0% and 255 for 100%
	OCR2A = 0;
	OCR2B = 0;
	
	// Configuration of the PWM setting for OC1A and OC1B
	// COM1n1:0 = 2 equals clear OC1A/B on compare match, and set at bottom
	// WGM12:0 = 5 equals 8-bit fast PWM mode
	// CS12:0 = 3 sets division factor to 64
	TCCR1A = 1<<COM1A1 | 0<<COM1A0 | 1<<COM1B1 | 0<<COM1B0 | 1<<WGM11 | 0<<WGM10;
	TCCR1B = 1<<WGM12 | 1<<CS12 | 0<<CS11 | 1<<CS10;
	
	// Set the compare values for the PWM, 0 == 0% and 255 for 100%
	OCR1A = 0;
	OCR1B = 0;
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
// 1 for forward, 0 for backward
void PWM_SetDirLeft(int dir)
{
	if (dir == 1)
	{
		PORTD &= ~(1 << PORTD0);
	}
	else if (dir == 0)
	{
		PORTD |= 1 << PORTD0;
	}
}
// 1 for forward, 0 for backward
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



// Setup a timer. Used by the D regulator.
void TIMER_init()
{
	TCCR0B = 1<<CS00 | 1<<CS01 | 0<<CS02; // Prescaler set to 64 --> 1 interrupt =  0.81875 ms
	TCNT0 = 0; // Initialize counter
	TIMSK0 = 1<<TOIE0; // Enable timer interrupts.
}

//----------------------------PWM----END-----------------------------


void Get_sensor_values() //Load all sensor values into sensor-array
{
	SPI_MasterTransmit(0,'s'); //value to be returned
	for (int8_t i = 1; i < 5; i++)
	{
		arrSensor[i - 1] = SPI_MasterTransmit(i,'s'); //load all 8 sensorvalues into sensor position [0-7]
	}
}
void Send_sensor_values() // Can combine with Get speed when in manual mode
{
	SPI_MasterTransmit(255,'k');
	for (int8_t i = 0; i < 3; i++)
	{
		SPI_MasterTransmit(arrSensor[i],'k');
	}
	if (MANUAL_MODE)
	{
		SPI_MasterTransmit((arrSensor[3] - 128),'k');
	}
	else SPI_MasterTransmit(arrSensor[3],'k');
}
void Get_speed_value()
{
	SPI_MasterTransmit(0b00000001, 'k');
	_delay_us(200);
	arrSpeed[0] = SPI_MasterTransmit(0x00,'k'); //Get left speed
	_delay_us(20);
	arrSpeed[1] = SPI_MasterTransmit(0x00,'k'); //Get right speed
	_delay_us(20);
	arrSpeed[2] = SPI_MasterTransmit(0x00,'k'); //Get left dir
	_delay_us(20);
	arrSpeed[3] = SPI_MasterTransmit(0x00,'k'); //Get right dir
	_delay_us(20);
	arrSpeed[4] = SPI_MasterTransmit(0x00,'k'); //Get gripclaw
}
ISR(TIMER0_OVF_vect)
{
	TIMER_overflows++;
	TIMER_wheel++;
	
	if (TIMER_overflows >= 10)
	{
		TIMER_overflows = 0;
		TIMER_overflows_deci++; 
	}
}

//----------------------------GYRO------------------------------------
unsigned char SPI_MasterTransmit_Gyro(unsigned char cData)
{
	// Load data into SPI data register.
	SPDR = cData;
	
	// Wait until transmission completes.
	while(!(SPSR & (1<<SPIF)));
	
	return SPDR;
}

void set_GyroSS_Low() // connect gyro
{
	PORTC &= ~(1<<PORTC0);
}
void set_GyroSS_High() // disconnect gyro
{
	PORTC |= (1<<PORTC0);
}
void Gyro_Init()
{

	SPCR &= ~(1<<DORD);
	int8_t high_byte;
	do{
		set_GyroSS_Low();
		SPI_MasterTransmit_Gyro(0b10010100); //Activate adc
		high_byte = SPI_MasterTransmit_Gyro(0x00); //Byte with EOC and Accepted instr. bit
		SPI_MasterTransmit_Gyro(0x00); //low byte
		set_GyroSS_High();
	} while ( (high_byte & 0b10000000) & !(high_byte& 0b00100000)); // IF EOC = 0 and acc.instr. = 1 we continue
	SPCR |= (1<<DORD);

}
void Gyro_StartConversion()
{
	int8_t high_byte;	
	SPCR &= ~(1<<DORD);
	do{
		set_GyroSS_Low();
		SPI_MasterTransmit_Gyro(0b10010100); //Activate adc, select angular rate 
		high_byte = SPI_MasterTransmit_Gyro(0x00); //Byte with EOC and Accepted instr. bit
		SPI_MasterTransmit_Gyro(0x00); //low byte
		set_GyroSS_High();

	} while (high_byte & 0b10000000); // IF  acc.instr. = 1 we continue
	SPCR |= (1<<DORD);

}
int16_t Gyro_PollResult()
{
	uint8_t high_byte, low_byte;
	uint16_t return_val;

	SPCR &= ~(1<<DORD);
	do{
		set_GyroSS_Low();
		SPI_MasterTransmit_Gyro(0b10000000); //Activate adc
		high_byte = SPI_MasterTransmit_Gyro(0x00); //Byte with EOC and Accepted instr. bit
		low_byte = SPI_MasterTransmit_Gyro(0x00); //low byte
		set_GyroSS_High();
	} while ((high_byte & 0b10000000) & !(high_byte & 0b00100000)); // IF EOC = 0 and acc.instr. = 1 we continue
	SPCR |= (1<<DORD);
	LCD_Clear();
	return_val = ((high_byte & 0x000f) << 8);
	return_val = return_val + low_byte;
	return_val = return_val >> 1;
	return return_val;
}

// Converts the adc reading to angles per second
int16_t adcToAngularRate(uint16_t adcValue)
{
	double AngularRate = (adcValue * 25/12)+400;  // in mV
	// from the data sheet, R2 gyroscope sensor version is 26,67 mV/deg
	// for gyroscope with max angular rate 300 deg/s
	//LCD_SetPosition(16);
	//LCD_display_uint16((uint16_t)AngularRate);
	int16_t retval_= (AngularRate - 2500)/6.67;
	return retval_;
}
void Gyro_Sleep()
{
	SPCR &= ~(1<<DORD);
	int8_t dataH;
	do {
		set_GyroSS_Low();
		SPI_MasterTransmit_Gyro(0b10010000);
		dataH = SPI_MasterTransmit_Gyro(0x00);
		SPI_MasterTransmit_Gyro(0x00);
		set_GyroSS_High();
	} while (dataH & 0b10000000);
	SPCR |= (1<<DORD);
}
int16_t Gyro_sequence()
{
	int16_t result = 0;
	Gyro_StartConversion();
	result = Gyro_PollResult();
	result = adcToAngularRate(result);
	LCD_SetPosition(0);
	LCD_display_int16(result);
	_delay_ms(250);
	return result;
}
// use only during rotation
// in functions MOTOR_RotateRight(), MOTOR_RotateLeft()
// stop rotating when 90 degrees reached; 

void checkAngle90()
{
	int16_t result;
	int16_t before = 0;
	do {
		result = 0;
		result = Gyro_sequence();	// 315us
		if (abs(result) > 120) 
		{
			result = before;
		}
		before = result;
		rotation_angle += result/6;  // /6
		LCD_Clear();
		LCD_SetPosition(0);
		LCD_display_int16(rotation_angle);
	} while (abs(rotation_angle) < 410); // 410
	
	rotation_angle = 0; //reset
}
//----------------------------GYRO----END-----------------------------
uint16_t wheel_marker_counter = 0;
uint16_t current_speed;
uint16_t wheel_circumference = 4*100*79; //Wheel circumference is 79 mm.
uint16_t time_difference;

void Speed_Interrupt_Init()
{
	EICRA = 1<< ISC00 | 1<<ISC01; //INT0 genererar avbrott p� rising flank
	EIMSK = 1<< INT0; //IINT0?
	//MCUCR = (1<<IVCE); //Boot flash?
	//MCUCR = (1<<IVSEL); //Boot flash?
}

ISR(INT0_vect)
{
	cli();
	LCD_SetPosition(24);
	LCD_display_uint16(wheel_marker_counter);
	_delay_ms(250);
	//EIMSK &= ~(1<INT0);
	if (wheel_marker_counter == 0)
	{
		TIMER_wheel = 0;
		wheel_marker_counter++;
	}
	else if (wheel_marker_counter < 8)
	{
		LCD_SetPosition(24);
		LCD_display_uint16(wheel_marker_counter);
		wheel_marker_counter++;
	}
	else
	{
		time_difference = TIMER_wheel / 0.8175; // div by 0.8175 -> time in ms
		current_speed = wheel_circumference / time_difference;
		LCD_Clear();
// 		LCD_SetPosition(0);
// 		LCD_display_int16(time_difference);
// 		LCD_SetPosition(8);
// 		LCD_display_int16(current_speed);
		LCD_SetPosition(24);
		LCD_display_uint16(wheel_marker_counter);
		wheel_marker_counter = 0; 
	}
	//EIMSK |= 1<INT0;
	sei();
}

//--MOTOR start
void MOTOR_Forward(int speed)
{
	PWM_SetDirRight(1);
	PWM_SetDirLeft(1);
	PWM_SetSpeedLeft(speed);
	PWM_SetSpeedRight(speed);
}
void MOTOR_Backward(int speed)
{
	PWM_SetDirRight(0);
	PWM_SetDirLeft(0);
	PWM_SetSpeedLeft(speed);
	PWM_SetSpeedRight(speed);
}
void MOTOR_RotateLeft()
{
	PWM_SetDirRight(0);
	PWM_SetDirLeft(1);
	PWM_SetSpeedLeft(100);
	PWM_SetSpeedRight(100);
	checkAngle90();
}
void MOTOR_RotateRight()
{
	PWM_SetDirRight(1);
	PWM_SetDirLeft(0);
	PWM_SetSpeedLeft(100);
	PWM_SetSpeedRight(100);
	checkAngle90();
}

void MOTOR_Stop()
{
	PWM_SetSpeedRight(0);
	PWM_SetSpeedLeft(0);
}
//--MOTOR stop

void Drive_test()
{
	//MOTOR_Forward(80);
// 	SERVO_LevelHigh();
// 	for(int i = 0; i <= 15; i++)
// 	{
// 		_delay_ms(250);
// 	}
	MOTOR_RotateRight();
	MOTOR_Stop();
	
	_delay_ms(5000);
	MOTOR_RotateLeft();
	MOTOR_Stop();
	_delay_ms(5000);
	
// 	SERVO_SetGrip();
// 	for(int i = 0; i <= 5; i++)
// 	{
// 		_delay_ms(250);
// 	}
// 	SERVO_LevelMid();
// 	MOTOR_Backward(80);
// 	for(int i = 0; i <= 15; i++)
// 	{
// 		_delay_ms(250);
// 	}
// 	SERVO_ReleaseGrip();
// 	MOTOR_RotateLeft();
// 	MOTOR_Stop();
// 	for(int i = 0; i <= 5; i++)
// 	{
// 		_delay_ms(250);
// 	}
// 	SERVO_LevelLow();
// 	MOTOR_Backward(80);
// 	for(int i = 0; i <= 32; i++)
// 	{
// 		_delay_ms(250);
// 	}
// 	SERVO_LevelMid();
// 	SERVO_SetGrip();
// 	MOTOR_Stop();
// 	for(int i = 0; i <= 5; i++)
// 	{
// 		_delay_ms(250);
// 	}
// 	SERVO_ReleaseGrip();
}
void Gyro_test()
{
	//DORD: Data order. Set to 0 to transmit MSB first, LSB last
	//OBS: Spegla det som skickas!
	
	//checkAngle90();
	int16_t result = 0;
	int16_t before = 0;
 	while (1)
 	{
 		LCD_Clear();
 		LCD_SetPosition(0);
 		result = Gyro_sequence(); //data ordningen sätts här
   		if(abs(result) > 120)
  		{
  			result = before;
  		}
		before = result;
		rotation_angle += result;
		LCD_display_int16(rotation_angle/30);
 	}
	return;
}
void Speed_test()
{
	MOTOR_Forward(80);
	LCD_Clear();
	LCD_SetPosition(0);
	LCD_SendString("v: ");
	LCD_SetPosition(6);
	LCD_display_uint16(current_speed);
}
void init_all()
{
	sleep_enable();	// Enable sleep instruction
	Styr_InitPortDirections();	// Initiate Port directions for the styrmodul.
	Styr_InitPortValues();	// Initiate Port Values for the styrmodul.
	SPI_MasterInit();	// Initiate the styrmodul as the SPI master.
	LCD_Init(); // Initiate the LCD.
	PWM_Init(); // Initiate PWM for motor
	TIMER_init(); // Initiate Timer settings
	LCD_WelcomeScreen(); // Welcomes the user with a nice message ;^)
	Gyro_Init();
	//Speed_Interrupt_Init(); ****************************************** KOMMENTERA IN, MEN FUNGERAR EJ ATT MANUELLSTYRA DÅ *****************************************
	sei();	// Enable global interrupts
}
void manual_drive()
{
	Get_speed_value();
	Get_sensor_values();
	PWM_SetSpeedLeft(arrSpeed[0]);
	PWM_SetSpeedRight(arrSpeed[1]);
	PWM_SetDirLeft(arrSpeed[2]);
	PWM_SetDirRight(arrSpeed[3]);
	if (arrSpeed[4] == 1)
	{
		SERVO_SetGrip();
	}
	else {
		SERVO_ReleaseGrip();
	}
	Send_sensor_values();
}

int main(void)
{
	init_all();

	
	LCD_Clear();
	while (1)
	{
	
		_delay_ms(10);
		while(AUTONOM_MODE)
		{
			Get_sensor_values();
			Send_sensor_values();
			LCD_Clear();
			LCD_SetPosition(2);
			LCD_SendString("AUTONOM_MODE");
			_delay_ms(200);
			
		}
		
		_delay_ms(10);
		while(MANUAL_MODE)
		{
			LCD_Clear();
			LCD_SetPosition(2);
			LCD_SendString("MANUAL_MODE");
			manual_drive();
			_delay_ms(200);
			
		}
		//Drive_test();
// 		Get_sensor_values();
// 		
// 		LCD_SetPosition(0);
// 		LCD_display_int8(arrSensor[0]);
// 		LCD_SetPosition(8);
// 		LCD_display_int8(arrSensor[1]);
// 		LCD_SetPosition(16);
// 		LCD_display_int8(arrSensor[2]);
// 		LCD_SetPosition(24);
// 		LCD_display_int8(arrSensor[3]);
// 		//
// 		Send_sensor_values();
// 		manual_drive();
		//Drive_test();
		//Gyro_test();
		//Drive_test();
		//Speed_test();
	}
	
}
