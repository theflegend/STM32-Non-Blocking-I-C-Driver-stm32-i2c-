# STM32-Non-Blocking-I-C-Driver-stm32-i2c-
非阻塞式的硬件i2c驱动代码，以单字节为单位帧进行控制，使用缓冲区/状态机概念管理数据，用户通过请求向中断发送控制命令，命令执行完毕后支持回调/反馈执行状态。支持多任务（可能吧）
奶奶的，本项目的作者被stm著名的硬件i2c折磨了很久，但是至少本项目证明这个东西没毛病，项目采用ai和人混合编写，项目驱动的核心还是我写的，毕竟ai不懂得思辨，但ai确实给出了很多debug的
方向，嵌入式的debug是ai目前很难完成的
说一些傻逼的坑点吧：
1.标准库函数void I2C_Send7bitAddress(I2C_TypeDef* I2Cx, uint8_t Address, uint8_t I2C_Direction)确实是发送7位数据，但是注意！发送的格式是这样的：
地址（7位）|控制位（1位），Address的值必须是数据手册里7位从机地址的值左移1位后才能填入，根据库原型：
void I2C_Send7bitAddress(I2C_TypeDef* I2Cx, uint8_t Address, uint8_t I2C_Direction)
{
  /* Check the parameters */
  assert_param(IS_I2C_ALL_PERIPH(I2Cx));
  assert_param(IS_I2C_DIRECTION(I2C_Direction));
  /* Test on the direction to set/reset the read/write bit */
  if (I2C_Direction != I2C_Direction_Transmitter)
  {
    /* Set the address bit0 for read */
    Address |= OAR1_ADD0_Set;
  }
  else
  {
    /* Reset the address bit0 for write */
    Address &= OAR1_ADD0_Reset;
  }
  /* Send the address */
  I2Cx->DR = Address;
}
可以发现这个函数只是在Address的最后1位根据transmit/receive指令在！替换！1/0，它没有帮你左移1位！
2.stm32的硬件i2c在一次执行中有很多事件，如果使用事件判断来推进执行，记得处理中间事件（比如ev8_2)，不然可能没有进入下一个执行阶段！
3.没了
