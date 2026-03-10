#ifndef MY_NONESTOP_I2C_H
#define MY_NONESTOP_I2C_H

#include "stm32f10x.h"                  // Device header
#include "Buffer.h"

#define I2C_DRV           I2C2
#define I2C_DRV_CLK       RCC_APB1Periph_I2C2
#define I2C_DRV_GPIO_CLK  RCC_APB2Periph_GPIOB
#define I2C_DRV_SCL_PIN   GPIO_Pin_10
#define I2C_DRV_SDA_PIN   GPIO_Pin_11
#define I2C_DRV_SCL_PORT  GPIOB
#define I2C_DRV_SDA_PORT  GPIOB
#define I2C_DRV_SPEED     1000000
#define I2C_NVIC_IRQChannel I2C2_EV_IRQn
#define Buffer_size 20


typedef enum {
	Start,
	End
}User_Command;

typedef enum {
	Empty,
	Finish,
	Working,
	
	Error
}I2C_Work_State;

typedef enum {
	Read,
	Write
}I2C_Command;

typedef enum {
	Feed_Back_None_Complete,
	Feed_Back_Complete,
	Feed_Back_Error,
}I2C_Feed_Back;


typedef struct {
	I2C_Command I2C_Com;
	uint16_t Device_Address;
	uint16_t Register_Address;
	uint8_t* Data_Send_Or_Receive;
	I2C_Feed_Back*Feed_Back;
	void* function;
}I2C_Data_Frame;

typedef struct{
	
	I2C_Data_Frame I2C_Data_Buffer[Buffer_size];
	CircularQueue I2C_Data_Queue;
	I2C_Data_Frame A_I2C_Data_Frame;
	I2C_Data_Frame Present_I2C_Data_Frame;
	I2C_Data_Frame Error_I2C_Data_Frame;
	
	I2C_Work_State State;
	User_Command user;
	
	uint8_t execulte_flag;
	uint8_t debug_Bit;
	
}I2C_Control;


extern I2C_Control I2C;
extern uint8_t I2C_Init_State;
extern I2C_Feed_Back I2C_Empty_Feedback;
void I2C_Callback_Empty(void);
void I2C_Drv_Init(void);
void I2C_Contorller(I2C_Control*I2C);
void I2C_Read_Or_Write(I2C_Control*I2C,I2C_Command I2C_Com,
	uint16_t Device_Address,
	uint16_t Register_Address,
	uint8_t* Data_Send_Or_Receive,
	I2C_Feed_Back*Feed_Back,
	void* function,
	uint8_t*execulte_state,
	uint8_t callback_flag);//callback_flag是防呆设计，0是空函数和空的feedback变量，1是启用了callback，这是提醒开发者为function赋好值的！函数里面不需要这个
	
#endif