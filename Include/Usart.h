/***********************************************************************
* @file           Usart.h
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    串口通讯模块
***********************************************************************/
#include "../Include/stm32g030xx.h"
#include "../Include/Define.h"

/*====================================================================*/
void UsartInit(void);
void UsartApp(void);
void SentReceData(void);
void UsartDmaRestart(void);

static void ReceDataProcess(void);
static void SendDataSetup(void);
/*====================================================================*/
tUsart_Type tUsart;
tVOFA_Type tVOFA;

extern tSystem_Type tSystem;
extern tProtect_Type tProtect;
extern tMotor_Type tMotor;
extern tBldc_Type tBldc;
extern tAdc_Type tAdc;
/*====================================================================*/
#define     USART_SET_MODE           0x0043
#define     USART_SET_DIRECTION      0x0044
#define     USART_SET_SPEEDMODE      0x0045
#define     USART_SET_DUTY           0x0046
#define     USART_SET_SPEED          0x0047

/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
