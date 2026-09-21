#include <can.h>
#include "stm32f4xx_hal.h"
CAN_HandleTypeDef hcan;
CAN_FilterTypeDef sFilterConfig;
GPIO_InitTypeDef gpio_init_struct;
void can_bsp_init()
{
	hcan.Instance=CAN1;
	hcan.Init.AutoBusOff=;
	hcan.Init.AutoRetransmission=;
	hcan.Init.AutoWakeUp=;
	hcan.Init.Mode=;
	hcan.Init.Prescaler=;
	hcan.Init.ReceiveFifoLocked=;
	hcan.Init.SyncJumpWidth=;
	hcan.Init.TimeSeg1=;
	hcan.Init.TimeSeg2=;
	hcan.Init.TimeTriggeredMode=;
	hcan.Init.TransmitFifoPriority=;
  HAL_CAN_Init(&hcan);
	
	sFilterConfig.FilterActivation=;
	sFilterConfig.FilterBank=;
	sFilterConfig.FilterFIFOAssignment=;
	sFilterConfig.FilterIdHigh=;
	sFilterConfig.FilterIdLow=;
	sFilterConfig.FilterMaskIdHigh=;
	sFilterConfig.FilterMaskIdLow=;
	sFilterConfig.FilterMode=;
	sFilterConfig.FilterScale=;
	sFilterConfig.SlaveStartFilterBank=;
	HAL_CAN_ConfigFilter(&hcan,&sFilterConfig);
	
	
	HAL_CAN_Start(&hcan);
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan){
	
gpio_init_struct.Alternate=;
gpio_init_struct.Mode=;

}