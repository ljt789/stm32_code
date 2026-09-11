#include "at24c02.h"
#include "./SYSTEM/usart/usart.h"
#include "stm32f4xx_hal.h"
I2C_HandleTypeDef hi2c1;
/**
@bridef  AT24C02对应的IIC驱动
**/
void AT24C02_BSP_INIT(){

  /*GPIO配置 */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef I2C1_GPIO;
	I2C1_GPIO.Mode=GPIO_MODE_AF_OD;
	I2C1_GPIO.Pin=GPIO_PIN_8|GPIO_PIN_9;
	I2C1_GPIO.Pull=GPIO_PULLUP;
	I2C1_GPIO.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	I2C1_GPIO.Alternate= GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &I2C1_GPIO);
	
  /*IIC1配置 */

	__HAL_RCC_I2C1_CLK_ENABLE();
	hi2c1.Instance=I2C1;
	hi2c1.Init.ClockSpeed=400000;
	hi2c1.Init.DutyCycle=I2C_DUTYCYCLE_2;
	hi2c1.Init.AddressingMode=I2C_ADDRESSINGMODE_7BIT;;
	hi2c1.Init.GeneralCallMode=I2C_GENERALCALL_DISABLE;;
	hi2c1.Init.NoStretchMode=I2C_NOSTRETCH_DISABLE;;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress1=0;
	hi2c1.Init.OwnAddress2=0;
	
if(HAL_I2C_Init(&hi2c1)!=HAL_OK){
	printf("I2C_INIT ERROR\r\n");
}
else{
	printf("I2C_INIT SUCCESS\r\n");
}
}
//PING AT24C02
void Ping_AT24C02(){
	
if (HAL_I2C_IsDeviceReady(&hi2c1, AT24C02_ADDR_WRITE, 3, 100) == HAL_OK)
{
    printf("AT24C02 ready\r\n");
}
else{
	printf("AT24C02 not ready\r\n");
}
}
//AT24C02总共256字节 共32页，每页8字节
void Write_byte_at24c02(uint8_t addr,uint8_t data){
	 HAL_I2C_Mem_Write(&hi2c1, AT24C02_ADDR_WRITE, (uint16_t)addr, sizeof(addr),&data, sizeof(data), 10);
}
void Read_byte_at24c02(){
}