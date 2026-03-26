/***********************************************************************
* @file           Motor.h
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    电机运行控制应用模块
***********************************************************************/
#include "../Include/stm32g030xx.h"
#include "../Include/Define.h"
#include "../Include/UserParams.h"

void MotorInit(void);
void MotorApp(void);
static void MotorRunStopControl(void);
static void CalcMotorSpeed(void);
static void MotorSetSpeed(unsigned short Var);
static unsigned short GetAimValue(unsigned short Var,unsigned short Aim,unsigned short MinValue,unsigned short MaxDelta);
static void MotorSpeedControl(void);
static void MotorSetDuty(unsigned short Var);
    
extern void PIInit(tPI_Type *PIStrcut,signed short Kp,signed short Ki,signed short Kc,signed short Max,signed short Min);
extern void PIControl(tPI_Type *PIStrcut);

extern unsigned short LowPassFilter(unsigned short K,unsigned short x,unsigned short y);

tMotor_Type tMotor;
tPI_Type tPI_Speed;

extern tAdc_Type tAdc;
extern tBldc_Type tBldc;
extern tSystem_Type tSystem;
extern tProtect_Type tProtect;
extern tUsart_Type tUsart;
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
