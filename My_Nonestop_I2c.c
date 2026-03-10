#include "stm32f10x.h"                  // Device header
#include "Buffer.h"
#include "My_Nonestop_I2c.h"
#include "OLED.h"
I2C_Control I2C;
uint8_t I2C_Init_State=1;
I2C_Feed_Back I2C_Empty_Feedback=0;

void I2C_Callback_Empty(void)
{
	
};

void I2C_Drv_Init(void)
{
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef i2c;
    NVIC_InitTypeDef nvic;

    /* 1. 时钟使能 */
    RCC_APB2PeriphClockCmd(I2C_DRV_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(I2C_DRV_CLK, ENABLE);

    /* 2. GPIO配置 */
    gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    gpio.GPIO_Pin = I2C_DRV_SCL_PIN;
    GPIO_Init(I2C_DRV_SCL_PORT, &gpio);

    gpio.GPIO_Pin = I2C_DRV_SDA_PIN;
    GPIO_Init(I2C_DRV_SDA_PORT, &gpio);

    /* 3. I2C外设复位 */
    I2C_DeInit(I2C_DRV);

    /* 4. I2C参数初始化 */
    memset(&i2c, 0, sizeof(i2c));

    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1 = 0x00;
    i2c.I2C_Ack = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_ClockSpeed = I2C_DRV_SPEED;

    I2C_Init(I2C_DRV, &i2c);

    /* 5. 开启I2C中断 */
    I2C_ITConfig(I2C_DRV, I2C_IT_EVT | I2C_IT_ERR | I2C_IT_BUF, ENABLE);

    /* 6. NVIC配置 - 事件中断 */
    memset(&nvic, 0, sizeof(nvic));

    nvic.NVIC_IRQChannel = I2C2_EV_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    /* 7. NVIC配置 - 错误中断 */
    nvic.NVIC_IRQChannel = I2C2_ER_IRQn;
    NVIC_Init(&nvic);

    /* 8. 使能I2C */
    I2C_Cmd(I2C_DRV, ENABLE);

    /* 9. 清可能存在的错误标志 */
    I2C_ClearFlag(I2C_DRV, I2C_FLAG_AF);
    I2C_ClearFlag(I2C_DRV, I2C_FLAG_ARLO);
    I2C_ClearFlag(I2C_DRV, I2C_FLAG_BERR);

    /* 10. 初始化软件控制结构 */
    if(I2C_Init_State)
    {
        memset(&I2C, 0, sizeof(I2C_Control));
        cq_init(&I2C.I2C_Data_Queue,
                I2C.I2C_Data_Buffer,
                Buffer_size,
                sizeof(I2C_Data_Frame));
        I2C_Init_State = 0;
    }
}
void I2C_Read_Or_Write(I2C_Control*I2C,I2C_Command I2C_Com,
	uint16_t Device_Address,
	uint16_t Register_Address,
	uint8_t* Data_Send_Or_Receive,
	I2C_Feed_Back*Feed_Back,
	void* function,
	uint8_t*execulte_state,
	uint8_t callback_flag)
//callback_flag是防呆设计，0是空函数和空的feedback变量，1是启用了callback和feedback，
	//这是提醒开发者为Feed_Back和function赋好值的！函数里面不需要这个
{
	I2C->user=Start;
	I2C->A_I2C_Data_Frame.I2C_Com=I2C_Com;
	I2C->A_I2C_Data_Frame.Data_Send_Or_Receive=Data_Send_Or_Receive;
	I2C->A_I2C_Data_Frame.Device_Address=Device_Address;
	I2C->A_I2C_Data_Frame.Feed_Back=Feed_Back;
	I2C->A_I2C_Data_Frame.function=function;
	I2C->A_I2C_Data_Frame.Register_Address=Register_Address;
	*I2C->A_I2C_Data_Frame.Feed_Back=0;
	if(cq_is_full(&I2C->I2C_Data_Queue))
	{
		*execulte_state=0;
	}
	else
	{
		cq_enqueue(&I2C->I2C_Data_Queue,&I2C->A_I2C_Data_Frame);
		*execulte_state=1;
	};
	
}//
		
void I2C_Contorller(I2C_Control*I2C)
{
	switch(I2C->user)
	{
		case Start:
			if(!cq_is_empty(&I2C->I2C_Data_Queue))
			{
				switch(I2C->State)
				{
					case Empty:
					case Finish:
						cq_dequeue(&I2C->I2C_Data_Queue,&I2C->Present_I2C_Data_Frame);
						I2C->State=Working;
						I2C_GenerateSTART(I2C_DRV, ENABLE);
						break;
					case Working:
						break;
					case Error:
						I2C_Drv_Init();
						//I2C_GenerateSTART(I2C_DRV, ENABLE);
						//I2C->execulte_flag=0;
						//I2C->execulte_flag=9;//为了调试直接终止运行。
						break;
				}
				break;
			}
			else
			{
				
			}
				
		case End:
			break;
	}
};

void I2C_execultor(I2C_Control*I2C)
{
		uint32_t event=I2C_GetLastEvent(I2C_DRV);
		I2C->debug_Bit++;
		switch(I2C->Present_I2C_Data_Frame.I2C_Com)
		{
			case Write:
				
				if(I2C->execulte_flag==0&&event==I2C_EVENT_MASTER_MODE_SELECT)
				{
					I2C->execulte_flag=1;
					
					I2C_Send7bitAddress(I2C_DRV, I2C->Present_I2C_Data_Frame.Device_Address, I2C_Direction_Transmitter);
					
				}
				else if(I2C->execulte_flag==1&&event==I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)
				{
					I2C->execulte_flag=2;
					I2C_SendData(I2C_DRV, I2C->Present_I2C_Data_Frame.Register_Address);
									
				}
				else if(I2C->execulte_flag==2&&event==I2C_EVENT_MASTER_BYTE_TRANSMITTING)
				{
					I2C->execulte_flag=3;
					I2C_SendData(I2C_DRV,*I2C->Present_I2C_Data_Frame.Data_Send_Or_Receive);	
				}
				else if(I2C->execulte_flag==3&&event==I2C_EVENT_MASTER_BYTE_TRANSMITTED)
				{
					I2C->execulte_flag=0;
					I2C->State=Finish;
					void (*func)(void) = (void (*)(void))I2C->Present_I2C_Data_Frame.function;
					func(); // 调用函数
					*I2C->Present_I2C_Data_Frame.Feed_Back=Feed_Back_Complete;
					I2C_GenerateSTOP(I2C_DRV, ENABLE);	
				}
				else
				{
					if(event!=I2C_EVENT_MASTER_BYTE_TRANSMITTED)
					{		
							I2C->State = Error;
							I2C->Error_I2C_Data_Frame=I2C->Present_I2C_Data_Frame;
							*I2C->Present_I2C_Data_Frame.Feed_Back=Feed_Back_Error;
							
					}
				}
				break;
			case Read:
				
				if(I2C->execulte_flag==0&&event==I2C_EVENT_MASTER_MODE_SELECT)
				{
					I2C->execulte_flag=1;
					I2C_Send7bitAddress(I2C_DRV, I2C->Present_I2C_Data_Frame.Device_Address<<1, I2C_Direction_Transmitter);
					I2C->debug_Bit++;
					
				}
				else if(I2C->execulte_flag==1&&event==I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)//
				{
					
					I2C->execulte_flag=2;
					I2C_SendData(I2C_DRV, I2C->Present_I2C_Data_Frame.Register_Address);
					I2C->debug_Bit++;
					
				}
				else if(I2C->execulte_flag==2&&event==I2C_EVENT_MASTER_BYTE_TRANSMITTED)
				{
					I2C->execulte_flag=3;
					I2C_GenerateSTART(I2C_DRV, ENABLE);	
					I2C->debug_Bit++;
				
				}
				else if(I2C->execulte_flag==3&&event==I2C_EVENT_MASTER_MODE_SELECT)
				{
					I2C->execulte_flag=4;
					I2C_Send7bitAddress(I2C_DRV, I2C->Present_I2C_Data_Frame.Device_Address<<1, I2C_Direction_Receiver);
					I2C->debug_Bit++;	
									
				}
				
				else if(I2C->execulte_flag==4 && event==I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)
				{
					

					I2C_AcknowledgeConfig(I2C_DRV, DISABLE);
					I2C_GenerateSTOP(I2C_DRV, ENABLE);  // 在这里 STOP
					I2C->debug_Bit++;
					
					// 不立刻 STOP
					I2C->execulte_flag = 5;
				}
				else if(I2C->execulte_flag==5&&event==I2C_EVENT_MASTER_BYTE_RECEIVED)
				{
					
					*I2C->Present_I2C_Data_Frame.Data_Send_Or_Receive= I2C_ReceiveData(I2C_DRV);											//接收数据寄存器
					*I2C->Present_I2C_Data_Frame.Feed_Back=Feed_Back_Complete;
					I2C->State=Finish;
					I2C->execulte_flag=0;
					I2C_AcknowledgeConfig(I2C_DRV, ENABLE);	
					void (*func)(void) = (void (*)(void))I2C->Present_I2C_Data_Frame.function;
							func(); // 调用函数]
				
				}
				
				else
				{
					if(event!=I2C_EVENT_MASTER_BYTE_TRANSMITTED)
					{		
							I2C->State = Error;
							I2C->Error_I2C_Data_Frame=I2C->Present_I2C_Data_Frame;
							*I2C->Present_I2C_Data_Frame.Feed_Back=Feed_Back_Error;
							
					}
				}
				break;
		}
	
}

void I2C_Error_execultor(I2C_Control*I2C)
{
	if(I2C_GetFlagStatus(I2C_DRV, I2C_FLAG_AF))
        I2C_ClearFlag(I2C_DRV, I2C_FLAG_AF);
    if(I2C_GetFlagStatus(I2C_DRV, I2C_FLAG_BERR))
        I2C_ClearFlag(I2C_DRV, I2C_FLAG_BERR);
    if(I2C_GetFlagStatus(I2C_DRV, I2C_FLAG_ARLO))
        I2C_ClearFlag(I2C_DRV, I2C_FLAG_ARLO);
	
    I2C->State = Error;
	I2C->Error_I2C_Data_Frame=I2C->Present_I2C_Data_Frame;
	*I2C->Present_I2C_Data_Frame.Feed_Back=Feed_Back_Error;
}
void I2C2_EV_IRQHandler(void)
{
	
	I2C_execultor(&I2C);
}
void I2C2_ER_IRQHandler(void)
{
	
	I2C_Error_execultor(&I2C);
	
}


