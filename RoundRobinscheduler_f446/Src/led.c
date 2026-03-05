#include "led.h"
#include "stm32f446xx.h"
#define GPIOAEN (1U<<0)
#define LED_PIN (1U<<5)
void led_init(void)
{
	//enable clock access to led port
	RCC->AHB1ENR |=GPIOAEN;
	//set led pin as output pin
	GPIOA->MODER |=(1U<<10);
	GPIOA->MODER &= ~(1U<<11);
}

void led_on(void)
{
	//set led pin HIGH(PA5)
	GPIOA->ODR |=LED_PIN;
}

void led_off(void)
{
	//set led pin LOW(PA5)
	GPIOA->ODR &= ~LED_PIN;
}
