/*
 * Final_arduino.c
 *
 * Created: 2018/12/4 18:10:38
 * Author : Wang
 */ 

#include <stdio.h>
#define F_CPU 200000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include "uart.h"
#include <inttypes.h>

#define TEST_PWM 10
#define TEST_PWM_1 25
#define TEST_PWM_2 50
#define TEST_PWM_3 75
#define TEST_PWM_BU 20
#define TEST_PWM_WIFI 30

#define TEST_RPM 140
#define TEST_RPM_ERR 122
#define TEST_RPM_OK 109
#define TEST_RPM_LED 80

uint16_t count1;
uint16_t count2;
uint16_t counter, balance = 0;
uint16_t holder;
uint16_t on=1;
uint16_t flag = 0;
//uint16_t comp_result;
volatile uint16_t start = 0;

unsigned int flag1 = 0,flag2 = 0,width = 0;
unsigned int width_a1,width_a2;
unsigned int period = 0, dutycycle = 0;
int i = 0;

uint16_t result_PWM=0,result_LED=0,result_BU=0,result_WIFI=0;

static void send_command1(void);
static void send_command2(void);
int main(void)
{
    /* Replace with your application code */
	//PORTD |= (1 << PORTD2);//use PD2 to test the LEDs
	DDRB |= (1<<PORTB2); 
	DDRB |= (1<<PORTB3); 
	DDRB |= (1<<PORTB4); //send to tachometer
	DDRB |= (1<<PORTB5); //led output for stable pwm
	//PORTD |= (1 << PORTD2);//use PD2 to test the LEDs
	//DDRD |= (1<<PORTD7);
	DDRD |= (1<<PORTD5);
	DDRD |= (1<<PORTD4);
	DDRB |= (1<<PORTB1); //led output for stable pwm
	
	TCCR0A |= (1<<WGM01);//ctc
	TCCR0B |= (1<<CS02) | (1<<CS00);//clk/1024
	TCCR0A |= (1 << COM0A0);
	OCR0A =	255;
	TIMSK0|= (1<<OCIE0A); //interrupt enable
	
	OCR1A = TCNT1 + 10;
	TIMSK1 |= (1 << ICIE1); // set edge interruption
	TIMSK1 |= (1 <<(TOIE1));
	TCCR1B |= (1 << ICES1); //rising edge
	TCCR1B |= (1 << CS12); // pre-scaler 1 bits
	
	EICRA |= (1 << ISC01) | (1 << ISC00);//INT0 interrupt on rising edge
	EIMSK |= (1 << INT0); //Enable INT0
	
	EICRA |= (1 << ISC11) | (1 << ISC10);//INT0 interrupt on rising edge
	//EIMSK |= (1 << INT1); //Enable INT0
	
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
	//Prescaler at 128 so we have an 125Khz clock source
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1); //Avcc(+5v) as voltage reference
	ADCSRB &= ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)); 
	ADCSRA |= (1<<ADATE);
	//Signal source, in this case is the free-running
	ADCSRA |= (1<<ADEN); //Power up the ADC
	ADCSRA |= (1<<ADSC); //Start converting
	
	counter = 0;
	uint16_t flag_tach=0,flag_b=0;
	uint16_t err_PWM=0;
	//send_command();
	
	sei(); //set i
	
	uart_init();
	
	PORTD = 0;
    while (1) 
    {
		PORTB |= (1 << PORTB5);
		PORTB &= ~(1 << PORTB1);
		PORTB &= ~(1 << PORTB2);
		PORTB &= ~(1 << PORTB3);
		//PORTB &= ~(1 << PORTB5);
		result_PWM = 0;
		result_LED = 0;
		result_BU = 0;
		result_WIFI = 0;
		start = 1;
		
		
		send_command1();
		//_delay_ms(10);
		//printf("%d\n",dutycycle);
		_delay_ms(5);
		//printf("%d\n",start);
		while(start == 1)
		{
			uint16_t k = 0;
			//_delay_ms(50);
			//PORTB |= (1 << PORTB5);
			//PORTB |= (1 << PORTB2);
			//PORTB |= (1 << PORTB3);
			
			while((dutycycle>TEST_PWM+2) || (dutycycle<TEST_PWM-2));
			OCR0A =	TEST_RPM;
			while(dutycycle==TEST_PWM);
			_delay_ms(50);
			if((dutycycle>TEST_PWM_1+5) || (dutycycle<TEST_PWM_1-5))
			{
				OCR0A =	TEST_RPM_ERR;
				err_PWM ++;
				_delay_ms(50);
			}
			else
			{
				OCR0A =	TEST_RPM_OK;
				PORTD |= (1<<PORTD5);
				_delay_ms(50);
				PORTD &= ~(1<<PORTD5);
				_delay_ms(50);
			}

			OCR0A =	TEST_RPM;
			//_delay_ms(50);
			while(dutycycle==TEST_PWM_1);

			
			if((dutycycle>TEST_PWM_2+3) || (dutycycle<TEST_PWM_2-3))
			{
				OCR0A =	TEST_RPM_ERR;
				err_PWM ++;
				_delay_ms(50);
			}
			else
			{
				OCR0A =	TEST_RPM_OK;
				PORTD |= (1<<PORTD5);
				_delay_ms(50);
				PORTD &= ~(1<<PORTD5);
				_delay_ms(50);
			}
			
			/*OCR0A =	TEST_RPM;
			_delay_ms(50);
			while(dutycycle==TEST_PWM_2);

			
			if((dutycycle>TEST_PWM_3+1) || (dutycycle<TEST_PWM_3-2))
			{
				OCR0A =	TEST_RPM_ERR;
				err_PWM ++;
				_delay_ms(50);
			}
			else
			{
				OCR0A =	TEST_RPM_OK;
				PORTD |= (1<<PORTD5);
				_delay_ms(50);
				PORTD &= ~(1<<PORTD5);
				_delay_ms(50);
			}*/
			
			
			
			if(err_PWM == 0)
			PORTD |= (1<<PORTD5);
			result_PWM = 1;
			
			
			
			OCR0A =	TEST_RPM_LED;
			while((dutycycle>TEST_PWM+1) || (dutycycle<TEST_PWM-2));
			PORTB |= (1 << PORTB1);
			result_LED = 1;
			
			
			while(k<2000)
			{

				//result_BU = 0;
				if(!((dutycycle>TEST_PWM_BU+2) || (dutycycle<TEST_PWM_BU-2)))
				{
					result_BU = 1;
					PORTB |= (1 << PORTB2);
					break;
				}
				k++;
				_delay_ms(1);
				//printf("%d\n",result_BU);
			}
			
			
			OCR0A =	TEST_RPM;
			//PORTB |= (1 << PORTB2);
			if(result_BU == 1)
			PORTB |= (1 << PORTB2);
			
			
			while((dutycycle>TEST_PWM_WIFI+3) || (dutycycle<TEST_PWM_WIFI-3));
			
			//_delay_ms(50);
			PORTB |= (1 << PORTB3);
			result_WIFI = 1;
			
			if(flag >= 5)
			{
				if(ADC >= 519)
				//result = 2;
				PORTB |= (1<<PORTB3);
				else
				//result = 1;
				PORTB &= ~(1<<PORTB3);
				//to do: send the result
				flag = 0;
				break;
			}
			
			PORTD |= (1 << PORTD4);
			
			_delay_ms(2000);
			PORTD &= ~(1 << PORTD4);
			start = 1;
			PORTD &= ~(1<<PORTD5);
			PORTB &= ~(1 << PORTB1);
			PORTB &= ~(1 << PORTB2);
			PORTB &= ~(1 << PORTB3);
			//PORTB &= ~(1 << PORTB5);
			result_PWM = 0;
			result_LED = 0;
			result_BU = 0;
			result_WIFI = 0;
			
			
		}
		
		
				
	}

}




ISR(TIMER1_CAPT_vect) // edge interruption
{

	if(TCCR1B &(1<< ICES1))
	{

		dutycycle = ((width_a2-67)*100/66)*100/93;
		flag1 = ICR1;
		TCCR1B &= ~(1 << ICES1);//falling edge
		//TIFR1 |= (1<<ICF1);

	}
	else
	{
		flag2 = ICR1;
		TCCR1B |= (1 << ICES1);//raising edge
		//record the width of echo
		width = flag2 - flag1;
		TCNT1 = 0x0000;
		i++;
		width_a1 +=width;
		if(i==50)
		{
			width_a2 =width_a1/50;
			i=0;
			width_a1=0;
			if(width_a2<67)
			width_a2 +=67;
		}

	}
	
	
}

ISR(TIMER0_COMPA_vect)
{
	PORTB ^= (1 << PORTB4); 
}

ISR(INT0_vect){
	
	//send_command1();
	EIFR &= ~(1 << INTF0); 
	start = 1;
	
	
}

static void send_command1()
{
		
		putchar('t');
		_delay_us(100);
		putchar('e');
		_delay_us(100);
		putchar('s');
		_delay_us(100);
		putchar('t');
		_delay_us(100);
		putchar('\r');
		_delay_us(100);
		//EIMSK &= ~(1 << INT0);

}


