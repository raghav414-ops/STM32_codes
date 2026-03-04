
#include <stdio.h>
#include <stdint.h>
#include "uart.h"
#include "stm32f446xx.h"
#define GPIOAEN (1U<<0)
#define UART2EN (1U<<17)
#define sys_freq 16000000
#define APB1_CLK sys_freq
#define UART_BAUDRATE 115200
#define CR1_TE (1U<<3)
#define CR1_UE (1U<<13)
#define SR_TXE (1U<<7)
static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate);
static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate);
static void uart_write(int ch);
int __io_putchar(int ch);

void uart_tx_init(void)
{
	//enable clock access to GPIOA
	RCC->AHB1ENR |=GPIOAEN;
	//set PA2 mode to alternate function mode
	GPIOA->MODER &= ~(1U<<4);
	GPIOA->MODER |= (1U<<5);
	//set alternate function type to AF7 (UART2_TX)
	GPIOA->AFR[0] |= (1U<<8);
	GPIOA->AFR[0] |= (1U<<9);
	GPIOA->AFR[0] |= (1U<<10);
	GPIOA->AFR[0] &= ~(1U<<11);
	//ENABLE CLOCK ACCESS TO UART
	RCC->APB1ENR |= UART2EN;
	//CONFIGURE BAUD RATE
	uart_set_baudrate(APB1_CLK, UART_BAUDRATE);
	USART2->CR1=CR1_TE;
	//ENABLE UART MODULE
	USART2->CR1|=CR1_UE;

}

int __io_putchar(int ch)
{
	uart_write(ch);
	return ch;

}

static void uart_write(int ch)
{
	//make sure the transmit data register is empty
	while(!(USART2->SR & SR_TXE)){}
	//write the transmit data register
	USART2->DR= (ch&0XFF);
}


static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate)
{
	USART2->BRR = compute_uart_bd(periph_clk, baudrate);
}

static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate)
{
	return ((periph_clk + (baudrate/2U))/baudrate);
}
