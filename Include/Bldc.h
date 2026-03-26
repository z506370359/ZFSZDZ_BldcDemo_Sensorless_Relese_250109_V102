/***********************************************************************
* @file           Bldc.h
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    电机控制应用代码模块
***********************************************************************/
#include "../Include/stm32g030xx.h"
#include "../Include/Define.h"
#include "../Include/UserParams.h"

void BldcStateMachine(void);
void Bldc_SetPwm_OnOff(unsigned char Phase);

unsigned char GetPhaseValue(unsigned char Phase,unsigned char Dir);
void AdcChannelSwitch(unsigned char Phase);
unsigned char GetSlope(unsigned char Phase,unsigned char Dir);

static void Bldc_Pwm_On_AB(void);
static void Bldc_Pwm_On_AC(void);
static void Bldc_Pwm_On_BC(void);
static void Bldc_Pwm_On_BA(void);
static void Bldc_Pwm_On_CA(void);
static void Bldc_Pwm_On_CB(void);
static void Bldc_Pwm_Off(void);
    
extern void IPDInit(void);
extern void Init_Position_Detect(void);

tBldc_Type tBldc;
extern tMotor_Type tMotor;
extern tAdc_Type tAdc;
extern tProtect_Type tProtect;

#define     STOP_STATE              (0)
#define     IPD_STATE               (1)
#define     ORIENT_INITFIRST_STATE  (2)
#define     ORIENT_INITSECOND_STATE (3)
#define     ORIENT_END_STATE        (4)
#define     FORCE_RUN_STATE         (5)
#define     SENSORLESS_RUN_STATE    (6)
#define     TEST_STATE              (7)
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
