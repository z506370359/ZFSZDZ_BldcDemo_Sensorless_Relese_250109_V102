/***********************************************************************
* @file           Main.h
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.10.12
* @Version        V1.0.0
* @description    张飞实战电子"扬帆起航"课程无感方波控制
***********************************************************************/
#include "../Include/stm32g030xx.h"
#include "../Include/Define.h"
#include "../Include/UserParams.h"

extern void Initialize(void);

extern void Display(void);

extern void AdcApp(void);
extern void AdcSample(void);
extern void AdcFilter(void);

extern void BldcStateMachine(void);

extern void MotorApp(void);

extern void Bldc_SetPwm_OnOff(unsigned char Phase);
extern unsigned char GetPhaseValue(unsigned char Phase,unsigned char Dir);
extern void AdcChannelSwitch(unsigned char Phase);
extern unsigned char GetSlope(unsigned char Phase,unsigned char Dir);

extern void HardOverCurrentMonitor(void);
extern void ProtectApp(void);

extern void SentReceData(void);
extern void UsartApp(void);
extern void UsartDmaRestart(void);

tSystem_Type tSystem;
extern tBldc_Type tBldc;
extern tMotor_Type tMotor;
extern tProtect_Type tProtect;

/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
