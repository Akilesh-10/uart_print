#include <stdint.h>
#include "GPIO_Driver.h"
#include "RCC_Driver.h"
#include "USART_Driver.h"

// This rx buffer is used in usart demo
uint8_t RxBuff[10];



USART_Struct_T Usart2Struct =
{
		.baudrate = 9600,
		.oversampling = 8,
		.parity = USART_PARITY_NONE,
		.stopbits = USART_STOPBIT_1,
		.usartId = USART2_ID,
		.wordlength = USART_WORDLENGTH_8B,
		.USARTInstance = USART2

};

void delay(void)
{
	for(uint32_t i=0; i < 1777777; i++ );
}

void led_demo()
{
	// Enable push button port A
	RCC_EnableGPIO(GPIOA);

	// Enable Led port clock - Port D
	RCC_EnableGPIO(GPIOD);

	GPIO_Init(GPIOD, &GreenLed);
	GPIO_Init(GPIOA, &PushButton);
	GPIO_Init(GPIOD, &OrangeLed);
	GPIO_Init(GPIOD, &RedLed);
	GPIO_Init(GPIOD, &BlueLed);
	//GPIO_SetPin(GPIOD, 14);
	while(1)
	{
		GPIO_TogglePin(GPIOD, 12);
		GPIO_TogglePin(GPIOD, 13);
		GPIO_TogglePin(GPIOD, 15);
		delay();
	}
}

void InterruptDemo()
{
	// Enable push button port A
	RCC_EnableGPIO(GPIOA);

	// Enable Led port clock - Port D
	RCC_EnableGPIO(GPIOD);

	// Initialize led pin
	GPIO_Init(GPIOD, &GreenLed);

	// Initialize pushbutton pin
	GPIO_Init(GPIOA, &PushButton);

	// Set the EXTI0 TO PA0 through sysconfig
	SYSCFG_SetEXTISource(0,0);

	// Enable EXTI Line interrupt
	EXTI_EnableInterrupt(0,EXTI_TRIGGER_RISING);

	// Enable NVIC Interrupt for EXTI0
	NVIC_EnableIRQ(EXTI0_IRQn);
}

void Uart_demo(void)
{
	const char *message = "Welcome to sir!\n";
	SCB_CPACR_ADDR |= (0xF << 20);

	RCC_Config_HSE_SystemClock();

	USART_Init(&Usart2Struct);

	USART_Transmit(&Usart2Struct, (uint8_t*)message, 19);

	while(1)
	{
		USART_Receive(&Usart2Struct, (uint8_t*)RxBuff, 1);
		USART_Transmit(&Usart2Struct, (uint8_t*)RxBuff, 1); // polling
	}
}

void Usart_partdemo(void)
{
	const char *message = "Welcome to sir!\n";
	SCB_CPACR_ADDR |= (0xF << 20);

	RCC_Config_HSE_SystemClock();

	USART_Init(&Usart2Struct);

	// enable Nvic module usart2 interrupt

	NVIC_EnableIRQ(USART2_IRQn);

	USART_Transmit_IT(&Usart2Struct, (uint8_t*)message, 19);
	USART_Receive_IT(&Usart2Struct, (uint8_t*)RxBuff, 1);

	while(1)
	{
		if (Usart2Struct.RxBusy == 0)
		{
			USART_Transmit_IT(&Usart2Struct, (uint8_t*)message, 19);
			USART_Receive_IT(&Usart2Struct, (uint8_t*)RxBuff, 1);
		}
	}

	while(1);
}

int main(void)
{
	// led_demo();
	//InterruptDemo();
	//Uart_demo();
	Usart_partdemo();

}


void USART2_IRQHandler()
{
	// transmit the data in interrupt
	// you should not use while loop do as fast as possible
	if((Usart2Struct.USARTInstance->SR & USARTx_SR_TXE) && (Usart2Struct.USARTInstance->CR1 & USARTx_CR1_TXEIE))
	{
		if(Usart2Struct.TxIndex < Usart2Struct.TxLength)
			Usart2Struct.USARTInstance->DR = Usart2Struct.pTxBuffer[Usart2Struct.TxIndex++];
		else
		{
			Usart2Struct.USARTInstance->CR1 &= ~(USARTx_CR1_TXEIE);
			Usart2Struct.TxBusy = 0;
		}
	}

	// Receive the data in interrupt mode
	if((Usart2Struct.USARTInstance->SR & USARTx_SR_RXNE) && (Usart2Struct.USARTInstance->CR1 & USARTx_CR1_RXNEIE))
	{

		Usart2Struct.pRxBuffer[Usart2Struct.RxIndex++] = Usart2Struct.USARTInstance->DR & 0xFF;

		if(Usart2Struct.RxIndex >= Usart2Struct.RxLength)
		{
			Usart2Struct.USARTInstance->CR1 &= ~(USARTx_CR1_RXNEIE);
			Usart2Struct.RxBusy = 0;
		}
	}
}