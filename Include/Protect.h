/***********************************************************************
* @file           Protect.h
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    故障监测及处理模块
***********************************************************************/
#include "../Include/stm32g030xx.h"
#include "../Include/Define.h"

void ProtectInit(void);
void ProtectApp(void);
void HardOverCurrentMonitor(void);

tProtect_Type tProtect;

extern tAdc_Type tAdc;

#define SOCTHRESHOLD    (496)//4A*0.02R*5/3.3*4096 = 496

#define OVTHRESHOLD     (300) //30V过压阈值
#define OVRETHRESHOLD   (290) //过压恢复阈值
#define LVTHRESHOLD     (210) //21V欠压阈值
#define LVRETHRESHOLD   (220) //欠压恢复阈值
#define OTTHRESHOLD     (60) //60摄氏度 过温阈值
#define OTRETHRESHOLD   (58) //过温恢复阈值
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
