/***********************************************************************
* @file           IPD.h
* @author         ZhaoYun(ZFSZDZ)
* @date           2025.01.10
* @Version        V1.0.0
* @description    转子初始位置检测
***********************************************************************/
#include "../Include/stm32g030xx.h"
#include "../Include/Define.h"
#include "../Include/UserParams.h"

void IPDInit(void);
void Init_Position_Detect(void);

tIPD_Type tIPD;
extern tAdc_Type tAdc;
extern tMotor_Type tMotor;
extern tBldc_Type tBldc;
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
