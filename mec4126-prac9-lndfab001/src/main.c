// Description----------------------------------------------------------------|
/*
 * Initialises a struct with Name and Age data. Displays results on LEDs and
 * LCD.
 */
// DEFINES AND INCLUDES-------------------------------------------------------|

#define STM32F051
//>>> Uncomment line 10 if using System Workbench (SW4STM32) or STM32CubeIDE
#define SW4STM32

#ifndef SW4STM32
	#define TRUESTUDIO
#endif

#include "stm32f0xx.h"

// GLOBAL VARIABLES ----------------------------------------------------------|


// FUNCTION DECLARATIONS -----------------------------------------------------|

void init_timer_2(void);
void init_timer_6(void);
void TIM6_IRQHandler(void);

void main(void);                                                   //COMPULSORY
void init_ADC(void);											   //COMPULSORY
#ifdef TRUESTUDIO												   //COMPULSORY
	void reset_clock_to_48Mhz(void);							   //COMPULSORY
#endif															   //COMPULSORY



// MAIN FUNCTION -------------------------------------------------------------|

void main(void)
{
#ifdef TRUESTUDIO  											 	   //COMPULSORY
	reset_clock_to_48Mhz();										   //COMPULSORY
#endif															   //COMPULSORY

	init_timer_2();
	init_timer_6();
	TIM6_IRQHandler();
	while(1)
	{

	}
}

// OTHER FUNCTIONS -----------------------------------------------------------|

#ifdef TRUESTUDIO												   //COMPULSORY
/* Description:
 * This function resets the STM32 Clocks to 48 MHz
 */
void reset_clock_to_48Mhz(void)									   //COMPULSORY
{																   //COMPULSORY
	if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL)			   //COMPULSORY
	{															   //COMPULSORY
		RCC->CFGR &= (uint32_t) (~RCC_CFGR_SW);					   //COMPULSORY
		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);	   //COMPULSORY
	}															   //COMPULSORY

	RCC->CR &= (uint32_t)(~RCC_CR_PLLON);						   //COMPULSORY
	while ((RCC->CR & RCC_CR_PLLRDY) != 0);						   //COMPULSORY
	RCC->CFGR = ((RCC->CFGR & (~0x003C0000)) | 0x00280000);		   //COMPULSORY
	RCC->CR |= RCC_CR_PLLON;									   //COMPULSORY
	while ((RCC->CR & RCC_CR_PLLRDY) == 0);						   //COMPULSORY
	RCC->CFGR |= (uint32_t) (RCC_CFGR_SW_PLL);					   //COMPULSORY
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);		   //COMPULSORY
}																   //COMPULSORY
#endif															   //COMPULSORY

void init_ADC(void)
{
	//enable port A
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	//enable PA6 to analogue mode
	GPIOA->MODER |= GPIO_MODER_MODER6;
	//enable clock to the ADC
	RCC->APB2ENR |= RCC_APB2ENR_ADCEN;
	//Enable ADC
	ADC1->CR |= ADC_CR_ADEN;
	//setup single shot mode
	ADC1->CFGR1 &= ~ADC_CFGR1_CONT;
	//Select channel 6
	ADC1->CHSELR |= ADC_CHSELR_CHSEL6;
	//Setup in wait mode
	ADC1->CFGR1 |= ADC_CFGR1_WAIT;
	//Setup 10 bit resolution
	ADC1->CFGR1 |= ADC_CFGR1_RES_1;
	//Wait for ADRDY flag to go high
	while((ADC1->ISR & ADC_ISR_ADRDY) == 0);
}

void init_timer_2(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;			//clock to timer
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;			//enable clock to PB
	GPIOB->MODER |= GPIO_MODER_MODER10_1;		//set B10 to alternate functions
	GPIOB->AFR[1] |= 0b1000000000;				//set AFR to AFR2 for PB10

	//set up for 15khz
	TIM2->PSC = 3;
	TIM2->ARR = 1023;
	//configure PWM
	TIM2->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE;
	//enable CC output
	TIM2->CCER |= TIM_CCER_CC3E;
	//enable timer
	TIM2->CR1 |= TIM_CR1_CEN;
	//Assigning duty cycle 100%
	TIM2->CCR3 = 1023;

}

void init_timer_6(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;			//enable clock for timer 6

	//50s delay overflow
	TIM6->PSC = 37;
	TIM6->ARR = 64865;

	TIM6->DIER |= TIM_DIER_UIE;					// enable interrupt
	NVIC_EnableIRQ(TIM6_DAC_IRQn);				// unmask TIM6 interrupt in NVIC
	TIM6->CR1 |= TIM_CR1_CEN;     				// enable Timer 6

}

// INTERRUPT HANDLERS --------------------------------------------------------|

void TIM6_IRQHandler(void)
{
	TIM6->SR &= ~TIM_SR_UIF;	//acknowledge interrupt
	ADC1->CR |= ADC_CR_ADSTART; //Start conversion
	while(((ADC1 -> ISR) &  ADC_ISR_EOC) == 0 ); //conversion complete with EOC flag in ISR goes high
	TIM2->CCR3 = ADC1->DR; //Save ADC value to TIM2
}
