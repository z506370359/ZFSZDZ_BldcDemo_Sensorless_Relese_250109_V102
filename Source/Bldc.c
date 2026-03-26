/***********************************************************************
* @file           Bldc.c
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    电机控制应用代码模块
***********************************************************************/
#include "../Include/Bldc.h"

/***********************************************************************
* 函数名称  : BldcStateMachine
* 功能描述  ：BLDC运行状态机
* 形参      ：无
* 返回值    ：无
* 要点备注  ：停止状态、预定位初始状态、预定位结束状态、强拖运行状态
              无感运行状态、测试状态
***********************************************************************/
void BldcStateMachine(void)
{
    signed short sTemp16;
    unsigned short Temp16;
    unsigned short DeventTempH;
    unsigned short DeventTempL;
    
    if(tBldc.PwmCount)
        tBldc.PwmCount --;
    
    switch(tBldc.State)
    {
        case STOP_STATE: //停止状态
            Bldc_SetPwm_OnOff(0);
            tBldc.PhaseStep = 0;
            
            TIM3->CNT = 0;
            TIM3->ARR = 65535;
            TIM3->SR &= 0xfffe; //UIF = 0
            TIM3->CR1 &= 0xfffe; //CEN = 0 关闭定时3
            
            tBldc.ZeventTimeNum = 0;
            tMotor.DisPlaySpeed = 0;
            tMotor.AimSpeed = 0;
            tMotor.ActSpeed = 0;
            tMotor.InstantSpeed = 0;
            
            tProtect.LockRotorPhaseCount = 1600;
            tProtect.LockRotorCurrentCount = 480;
            
            tBldc.SpeedUpdate = 0;
            tBldc.SpeedEnable = 0;
            
            IPDInit();
            
            break;
        case IPD_STATE: //转子初始位置检测状态
            Init_Position_Detect();
            break;
        case ORIENT_INITFIRST_STATE: //启动初始定位状态
            tBldc.Duty = ORIENT_DUTY; //设定定位占空比
            tBldc.PwmCount = ORIENT_TIME; //设定定位时间
            Bldc_SetPwm_OnOff(tBldc.PhaseStep); //给绕组通电
            tBldc.State = ORIENT_END_STATE;
            break;
        case ORIENT_END_STATE: //预定位结束状态
            if(tBldc.PwmCount == 0) //设定的预定位时间到了
            {
                if(tMotor.Direction == DIR_CCW) //相位初始值设置
                {
                    if(tBldc.PhaseStep == 6)
                        tBldc.PhaseStep = 1;
                    else
                        tBldc.PhaseStep ++;
                }
                else
                {
                    if(tBldc.PhaseStep == 1)
                        tBldc.PhaseStep = 6;
                    else
                        tBldc.PhaseStep --;
                }
                tBldc.DemagnetizationTime = 3;//转速高 电流大，设置退磁时间长 
                                              //转速低 电流小，设置退磁时间短
                
                tBldc.Duty = DUTY_MIN; //这是升频升压最小占空比
                
                tBldc.ForcePhaseTimeCount = 0;
                tBldc.ForcePhaseTimeBase = FORCEPHASETIME_MAX; //设定换相时间最大值
                tBldc.TerminalVoltageMax = 0;
                tBldc.TerminalVoltageMin = 2708;
                tBldc.ZeroPointCount = 0;
                
                tBldc.ForceState = 1;
                TEST_PIN_LOW;
                
                //直接无感
//                tBldc.PhaseStep = GetPhaseValue(tBldc.PhaseStep,tMotor.Direction);
//                tBldc.Slope = GetSlope(tBldc.PhaseStep,tMotor.Direction);
//                Bldc_SetPwm_OnOff(tBldc.PhaseStep); //给绕组通电
//                AdcChannelSwitch(tBldc.PhaseStep); //端电压AD采样通道切换
//                tBldc.DemagnetizationTime = 3;//转速高 电流大，设置退磁时间长 
//                                          //转速低 电流小，设置退磁时间短
//                tBldc.SensorlessState = 1;
//                TIM14->CNT = 0;
//                tBldc.State = SENSORLESS_RUN_STATE;

                //需要先升频升压    
                tBldc.State = FORCE_RUN_STATE;
            }
            break;
        case FORCE_RUN_STATE: //强拖运行状态
            tBldc.ForcePhaseTimeCount++;
            switch(tBldc.ForceState)
            {
                case 1: //按照相位值换相
                    tBldc.PhaseStep = GetPhaseValue(tBldc.PhaseStep,tMotor.Direction);
                    tBldc.Slope = GetSlope(tBldc.PhaseStep,tMotor.Direction);
                    Bldc_SetPwm_OnOff(tBldc.PhaseStep); //给绕组通电
                    AdcChannelSwitch(tBldc.PhaseStep); //端电压AD采样通道切换
                    tBldc.ForcePhaseTimeCount = 0;
                    tBldc.ForceState = 2;
                    break;
                case 2: //等待换相续流结束
                    //自动检测退磁事件 D事件
                    tBldc.TerminalVoltage = tAdc.TerminalVoltage;
                    DeventTempH = (tAdc.Vbus >> 1) + (tAdc.Vbus >> 4);
                    DeventTempL = tAdc.Vbus >> 8;
                    
                    if(tBldc.Slope)
                    {
                        if(tBldc.TerminalVoltage < DeventTempH)
                        {
                            tBldc.ForceState = 3;
                            //TEST_PIN_LOW;
                        }
                    }
                    else
                    {
                        if(tBldc.TerminalVoltage > DeventTempL)
                        {
                            //TEST_PIN_HIGH;
                            tBldc.ForceState = 3;
                        }
                    }
                    //传统设置避开多个PWM周期 实现退磁检测
//                    if(tBldc.DemagnetizationTime)
//                        tBldc.DemagnetizationTime --;
//                    if(tBldc.DemagnetizationTime == 0)
//                    {
//                        tBldc.DemagnetizationTime = 3;
//                        tBldc.ForceState = 3;
//                    }
                    break;
                case 3: //按照Phase 检测反电动势过零点
                    tBldc.TerminalVoltage = tAdc.TerminalVoltage;
                    tBldc.VbusZcp = (tAdc.Vbus * BEMF_ZERO_FACTOR) >> 16; //1/2Vbus
                    if(tBldc.Slope)
                    {
                        if(tBldc.TerminalVoltage >= tBldc.VbusZcp) //过零点
                        {
                            //TEST_PIN_HIGH;
                            if(tBldc.TerminalVoltage < tBldc.TerminalVoltageMin) //更新阈值
                            {
                                tBldc.TerminalVoltageMin = tBldc.TerminalVoltage;
                            }
                            sTemp16 = tBldc.TerminalVoltage - tBldc.TerminalVoltageMin;
                            Temp16 = (tAdc.Vbus * PHASE_N_FACTOR) >> 16; //Vbus * N% 5%
                            
                            if(sTemp16 >= Temp16) //换相
                            {
                                tBldc.ForceState = 1;
                                tBldc.ZeroPointCount ++;
                            }
                        }
                    }
                    else
                    {
                        if(tBldc.TerminalVoltage <= tBldc.VbusZcp) //过零点
                        {
                            //TEST_PIN_LOW;
                            if(tBldc.TerminalVoltage > tBldc.TerminalVoltageMax) //更新阈值
                            {
                                tBldc.TerminalVoltageMax = tBldc.TerminalVoltage;
                            }
                            sTemp16 = tBldc.TerminalVoltageMax - tBldc.TerminalVoltage;
                            Temp16 = (tAdc.Vbus * PHASE_N_FACTOR) >> 16; //Vbus * N% 5%
                            
                            if(sTemp16 >= Temp16) //换相
                            {
                                tBldc.ForceState = 1;
                            }
                        }
                    }
                    
                    if(tBldc.ZeroPointCount >= 2)
                        tBldc.ForceState = 5;
                        
                    if(tBldc.ForcePhaseTimeCount > tBldc.ForcePhaseTimeBase)
                        tBldc.ForceState = 4;
                    
                    break;
                case 4://升频(减小换相时间)升压(增大PWM占空比)
                    tBldc.Duty += DUTY_STEP;  //升压
                    if(tBldc.Duty > DUTY_MAX)
                        tBldc.Duty = DUTY_MAX;
                    
                    tBldc.ForcePhaseTimeBase -= FORCEPHASETIME_STEP;  //升频
                    if(tBldc.ForcePhaseTimeBase < FORCEPHASETIME_MIN)
                        tBldc.ForcePhaseTimeBase = FORCEPHASETIME_MIN;
                    
                    tBldc.ForceState = 1;
                    tBldc.ZeroPointCount = 0;
                    break;
                case 5:
                    tBldc.PhaseStep = GetPhaseValue(tBldc.PhaseStep,tMotor.Direction);
                    tBldc.Slope = GetSlope(tBldc.PhaseStep,tMotor.Direction);
                    Bldc_SetPwm_OnOff(tBldc.PhaseStep); //给绕组通电
                    AdcChannelSwitch(tBldc.PhaseStep); //端电压AD采样通道切换
                    tBldc.DemagnetizationTime = 3;//转速高 电流大，设置退磁时间长 
                                              //转速低 电流小，设置退磁时间短
                    tBldc.SensorlessState = 1;
                    TIM14->CNT = 0;
                    tBldc.State = SENSORLESS_RUN_STATE;
                    break;
                default:
                    tBldc.State = STOP_STATE;
                    break;
            }
            //强拖启动失败检测
            if((tBldc.Duty >= DUTY_MAX) && (tBldc.ForcePhaseTimeBase <= FORCEPHASETIME_MIN))    
            {
                tProtect.StartFailCount ++;
                if(tProtect.StartFailCount >= 16000) //1s
                {
                    tProtect.StartFailCount = 0;
                    if((tProtect.Code == 0) || (tProtect.Code > 7))
                    {
                        tProtect.Code = 8;
                        //关闭PWM输出
                        TIM1->CCER = 0x00001ddd;
                        TIM1->CCMR1 = 0x00004848;
                        TIM1->CCMR2 = 0x00006848;
                        
                        TIM1->CCR1 = 0;
                        TIM1->CCR2 = 0;
                        TIM1->CCR3 = 0;
                    }
                }
            }
            break;
        case SENSORLESS_RUN_STATE: //无感运行状态
            switch(tBldc.SensorlessState)
            {
                case 1: //等待退磁结束(换相续流) 退磁事件自动检测
                    //自动检测退磁事件 D事件
                    tBldc.TerminalVoltage = tAdc.TerminalVoltage;
                    DeventTempH = (tAdc.Vbus >> 1) + (tAdc.Vbus >> 4);
                    DeventTempL = tAdc.Vbus >> 8;
                    
                    if(tBldc.Slope)
                    {
                        if(tBldc.TerminalVoltage < DeventTempH)
                        {
                            tBldc.SensorlessState = 2;
                            //TEST_PIN_LOW;
                        }
                    }
                    else
                    {
                        if(tBldc.TerminalVoltage > DeventTempL)
                        {
                            //TEST_PIN_HIGH;
                            tBldc.SensorlessState = 2;
                        }
                    }
                    //传统设置多个PWM周期 避开退磁事件
//                    if(tBldc.DemagnetizationTime)
//                        tBldc.DemagnetizationTime --;
//                    if(tBldc.DemagnetizationTime == 0)
//                    {
//                        tBldc.DemagnetizationTime = 8;
//                        tBldc.SensorlessState = 2;
//                    }
                    break;
                case 2: //检测反电动势过零点，延时30度电角度换相
                    tBldc.TerminalVoltage = tAdc.TerminalVoltage;
                    tBldc.VbusZcp = (tAdc.Vbus * BEMF_ZERO_FACTOR) >> 16; //1/2Vbus
                    if(tBldc.Slope) //上坡
                    {
                        if(tBldc.TerminalVoltage >= tBldc.VbusZcp)
                        {
                            TEST_PIN_LOW;
                            TIM14->CR1 &= 0xfffe; //CEN = 0 关闭定时14
                            tBldc.ZeventTime = TIM14->CNT; //反电动势过零点跟过零点的时间间隔
                            TIM14->CNT = 0;
                            TIM14->CR1 |= 0x0001; //CEN = 1 使能定时14
                            
                            tBldc.CeventTime = tBldc.ZeventTime >> 1;
                            
                            TIM3->CNT = 0;
                            TIM3->ARR = tBldc.CeventTime;
                            TIM3->SR &= 0xfffe; //UIF = 0
                            TIM3->CR1 |= 0x0001; //CEN = 1 使能定时3
                            
                            //存储过零点时间间隔 60度时间
                            tBldc.ZeventTimeArray[tBldc.ZeventTimeNum ++] = tBldc.ZeventTime;
                            if(tBldc.ZeventTimeNum >= 6)
                            {
                                tBldc.ZeventTimeNum = 0;
                                tBldc.SpeedUpdate = 1;
                                tBldc.SpeedEnable = 1;
                            }
                            tBldc.SensorlessState = 3;
                        }
                    }
                    else //下坡
                    {
                        if(tBldc.TerminalVoltage <= tBldc.VbusZcp)
                        {
                            TEST_PIN_HIGH;
                            TIM14->CR1 &= 0xfffe; //CEN = 0 关闭定时14
                            tBldc.ZeventTime = TIM14->CNT;
                            TIM14->CNT = 0;
                            TIM14->CR1 |= 0x0001; //CEN = 1 使能定时14
                            
                            tBldc.CeventTime = tBldc.ZeventTime >> 1;
                            
                            TIM3->CNT = 0;
                            TIM3->ARR = tBldc.CeventTime;
                            TIM3->SR &= 0xfffe; //UIF = 0
                            TIM3->CR1 |= 0x0001; //CEN = 1 使能定时3
                            
                            //存储过零点时间间隔 60度时间
                            tBldc.ZeventTimeArray[tBldc.ZeventTimeNum ++] = tBldc.ZeventTime;
                            if(tBldc.ZeventTimeNum >= 6)
                            {
                                tBldc.ZeventTimeNum = 0;
                                tBldc.SpeedUpdate = 1;
                                tBldc.SpeedEnable = 1;
                            }
                            
                            tBldc.SensorlessState = 3;
                        }
                    }
                    break;
                case 3: //等待换相状态
                    tBldc.Duty = tBldc.AimDuty;
                    break;
                default:
                    break;
            }
            //堵转电流保护  30ms不换相且电流大于2A
            if(tProtect.LockRotorCurrentCount)
            {
                tProtect.LockRotorCurrentCount --;
            }
            else
            {
                if(tAdc.An4.ValueAverage > 248)
                {
                    if((tProtect.Code == 0) || (tProtect.Code > 1))
                    {
                        tProtect.Code = 2;
                        //关闭PWM输出
                        TIM1->CCER = 0x00001ddd;
                        TIM1->CCMR1 = 0x00004848;
                        TIM1->CCMR2 = 0x00006848;
                        
                        TIM1->CCR1 = 0;
                        TIM1->CCR2 = 0;
                        TIM1->CCR3 = 0;
                    }
                }
            }
            //堵转相位保护 100ms不换相 堵转相位保护
            if(tProtect.LockRotorPhaseCount)
            {
                tProtect.LockRotorPhaseCount --;
            }
            else
            {
                if((tProtect.Code == 0) || (tProtect.Code > 2))
                {
                    tProtect.Code = 3;
                    //关闭PWM输出
                    TIM1->CCER = 0x00001ddd;
                    TIM1->CCMR1 = 0x00004848;
                    TIM1->CCMR2 = 0x00006848;
                    
                    TIM1->CCR1 = 0;
                    TIM1->CCR2 = 0;
                    TIM1->CCR3 = 0;
                }
            }
            break;
        case TEST_STATE: //测试状态
            break;
        default:
            break;
    }
}
/***********************************************************************
* 函数名称  : GetPhaseValue
* 功能描述  ：获取换相相位值
* 形参      ：Phase 相序值  Dir 电机的旋转方向
* 返回值    ：Phase 新的相序值
* 要点备注  ：无
***********************************************************************/
unsigned char GetPhaseValue(unsigned char Phase,unsigned char Dir)
{
    if(Dir == DIR_CCW)
    {
        Phase ++;
        if(Phase > 6)
            Phase = 1;
        if(Phase < 1)
            Phase = 6;
    }
    else if(Dir == DIR_CW)
    {
        Phase --;
        if(Phase < 1)
            Phase = 6;
        if(Phase > 6)
            Phase = 1;
    }
    return (Phase);
}
/***********************************************************************
* 函数名称  : GetSlope
* 功能描述  ：获取悬空相端电压变化趋势
* 形参      ：Phase 相序值  Dir 电机的旋转方向
* 返回值    ：Slope ：1对应上坡   0对应下坡
* 要点备注  ：无
***********************************************************************/
unsigned char GetSlope(unsigned char Phase,unsigned char Dir)
{
    unsigned char Slope;
    
    if(Dir == DIR_CCW)
    {
        if((Phase == 1) || (Phase == 3) || (Phase == 5))
        {
            Slope = 0; //下坡
            //TEST_PIN_LOW;
        }
        else if((Phase == 2) || (Phase == 4) || (Phase == 6))
        {
            Slope = 1; //上坡
            //TEST_PIN_HIGH;
        }
    }
    else
    {
        if((Phase == 1) || (Phase == 3) || (Phase == 5))
        {
            Slope = 1; //上坡
        }
        else if((Phase == 2) || (Phase == 4) || (Phase == 6))
        {
            Slope = 0; //下坡
        }
    }
    return (Slope);
}
/***********************************************************************
* 函数名称  : Bldc_SetPwm_OnOff
* 功能描述  ：根据相位值给绕组通电
* 形参      ：Phase 相序值
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void Bldc_SetPwm_OnOff(unsigned char Phase)
{
    switch(Phase)
    {
        case 1:
            Bldc_Pwm_On_AB();
            break;
        case 2:
            Bldc_Pwm_On_AC();
            break;
        case 3:
            Bldc_Pwm_On_BC();
            break;
        case 4:
            Bldc_Pwm_On_BA();
            break;
        case 5:
            Bldc_Pwm_On_CA();
            break;
        case 6:
            Bldc_Pwm_On_CB();
            break;
        default:
            Bldc_Pwm_Off();
            break;
    }
}
/***********************************************************************
* 函数名称  : AdcChannelSwitch
* 功能描述  ：悬空相AD采样通道切换
* 形参      ：Phase 相序值
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void AdcChannelSwitch(unsigned char Phase)
{
    switch(Phase)
    {
        case 1:
            ADC1->CHSELR = 0x00f95432;  //C相为悬空相
            break;
        case 2:
            ADC1->CHSELR = 0x00f95431;  //B相为悬空相
            break;
        case 3:
            ADC1->CHSELR = 0x00f95430;  //A相为悬空相
            break;
        case 4:
            ADC1->CHSELR = 0x00f95432;  //C相为悬空相
            break;
        case 5:
            ADC1->CHSELR = 0x00f95431;  //B相为悬空相
            break;
        case 6:
            ADC1->CHSELR = 0x00f95430;  //A相为悬空相
            break;
        default:
            break;
    }
}
/***********************************************************************
* 函数名称  : Bldc_Pwm_On_AB
* 功能描述  ：A(U)相绕组跟B(V)相绕组通电
* 形参      ：无
* 返回值    ：无
* 要点备注  ：电流从A相流进，从B相流出
***********************************************************************/
static void Bldc_Pwm_On_AB(void)
{
    TIM1->CCER = 0x00001041; //CC1E = 1  CC2NE = 1 CC4E = 1
    TIM1->CCMR1 = 0x00005868;//OC1CE = 0 OC1M[2:0] = 0b110 pwm mode 1 OC1PE = 1 OC1FE = 0 CC1S[1:0] = 0b00 OUT
                             //OC2CE = 0 OC2M[2:0] = 0b101 Force active level - OC1REF is forced high. OC2PE = 1 OC2FE = 0 CC2S[1:0] = 0b00 OUT
    TIM1->CCMR2 = 0x00006800; //OC3CE = 0 OC3M[2:0] = 0b000 Frozen OC3PE = 0 OC3FE = 0 CC3S[1:0] = 0b00 OUT
                              //OC4CE = 0 OC4M[2:0] = 0b110 pwm mode 1 OC4PE = 1 OC4FE = 0 CC4S[1:0] = 0b00 OUT
    TIM1->CCR1 = tBldc.Duty;
    TIM1->CCR2 = tBldc.Duty;
    TIM1->CCR3 = tBldc.Duty;
}
/***********************************************************************
* 函数名称  : Bldc_Pwm_On_AC
* 功能描述  ：A(U)相绕组跟C(W)相绕组通电
* 形参      ：无
* 返回值    ：无
* 要点备注  ：电流从A相流进，从C相流出
***********************************************************************/
static void Bldc_Pwm_On_AC(void)
{
    TIM1->CCER = 0x00001401; //CC1E = 1  CC3NE = 1 CC4E = 1
    TIM1->CCMR1 = 0x00000068;//OC1CE = 0 OC1M[2:0] = 0b110 pwm mode 1 OC1PE = 1 OC1FE = 0 CC1S[1:0] = 0b00 OUT
                             //OC2CE = 0 OC2M[2:0] = 0b000 Frozen OC2PE = 0 OC2FE = 0 CC2S[1:0] = 0b00 OUT
    TIM1->CCMR2 = 0x00006858; //OC3CE = 0 OC3M[2:0] = 0b101 Force active level - OC1REF is forced high. OC3PE = 1 OC3FE = 0 CC3S[1:0] = 0b00 OUT
                              //OC4CE = 0 OC4M[2:0] = 0b110 pwm mode 1 OC4PE = 1 OC4FE = 0 CC4S[1:0] = 0b00 OUT
    TIM1->CCR1 = tBldc.Duty;
    TIM1->CCR2 = tBldc.Duty;
    TIM1->CCR3 = tBldc.Duty;
}
/***********************************************************************
* 函数名称  : Bldc_Pwm_On_BC
* 功能描述  ：B(V)相绕组跟C(W)相绕组通电
* 形参      ：无
* 返回值    ：无
* 要点备注  ：电流从B相流进，从C相流出
***********************************************************************/
static void Bldc_Pwm_On_BC(void)
{
    TIM1->CCER = 0x00001410; //CC2E = 1  CC3NE = 1 CC4E = 1
    TIM1->CCMR1 = 0x00006800;//OC2CE = 0 OC2M[2:0] = 0b110 pwm mode 1 OC2PE = 1 OC2FE = 0 CC2S[1:0] = 0b00 OUT
                             //OC1CE = 0 OC1M[2:0] = 0b000 Frozen OC1PE = 0 OC1FE = 0 CC1S[1:0] = 0b00 OUT
    TIM1->CCMR2 = 0x00006858; //OC3CE = 0 OC3M[2:0] = 0b101 Force active level - OC1REF is forced high. OC3PE = 1 OC3FE = 0 CC3S[1:0] = 0b00 OUT
                              //OC4CE = 0 OC4M[2:0] = 0b110 pwm mode 1 OC4PE = 1 OC4FE = 0 CC4S[1:0] = 0b00 OUT
    TIM1->CCR1 = tBldc.Duty;
    TIM1->CCR2 = tBldc.Duty;
    TIM1->CCR3 = tBldc.Duty;
}
/***********************************************************************
* 函数名称  : Bldc_Pwm_On_BA
* 功能描述  ：B(V)相绕组跟A(U)相绕组通电
* 形参      ：无
* 返回值    ：无
* 要点备注  ：电流从B相流进，从A相流出
***********************************************************************/
static void Bldc_Pwm_On_BA(void)
{
    TIM1->CCER = 0x00001014; //CC2E = 1  CC1NE = 1 CC4E = 1
    TIM1->CCMR1 = 0x00006858;//OC2CE = 0 OC2M[2:0] = 0b110 pwm mode 1 OC2PE = 1 OC2FE = 0 CC2S[1:0] = 0b00 OUT
                             //OC1CE = 0 OC1M[2:0] = 0b101 Force active level - OC1REF is forced high. OC1PE = 1 OC1FE = 0 CC1S[1:0] = 0b00 OUT
    TIM1->CCMR2 = 0x00006800; //OC3CE = 0 OC3M[2:0] = 0b000 Frozen OC3PE = 0 OC3FE = 0 CC3S[1:0] = 0b00 OUT
                              //OC4CE = 0 OC4M[2:0] = 0b110 pwm mode 1 OC4PE = 1 OC4FE = 0 CC4S[1:0] = 0b00 OUT
    TIM1->CCR1 = tBldc.Duty;
    TIM1->CCR2 = tBldc.Duty;
    TIM1->CCR3 = tBldc.Duty;
}
/***********************************************************************
* 函数名称  : Bldc_Pwm_On_CA
* 功能描述  ：C(W)相绕组跟A(U)相绕组通电
* 形参      ：无
* 返回值    ：无
* 要点备注  ：电流从C相流进，从A相流出
***********************************************************************/
static void Bldc_Pwm_On_CA(void)
{
    TIM1->CCER = 0x00001104; //CC3E = 1  CC1NE = 1 CC4E = 1
    TIM1->CCMR1 = 0x00000058;//OC2CE = 0 OC2M[2:0] = 0b000 Frozen OC2PE = 0 OC2FE = 0 CC2S[1:0] = 0b00 OUT
                             //OC1CE = 0 OC1M[2:0] = 0b101 Force active level - OC1REF is forced high. OC1PE = 1 OC1FE = 0 CC1S[1:0] = 0b00 OUT
    TIM1->CCMR2 = 0x00006868; //OC3CE = 0 OC3M[2:0] = 0b110 pwm mode 1 OC3PE = 1 OC3FE = 0 CC3S[1:0] = 0b00 OUT
                              //OC4CE = 0 OC4M[2:0] = 0b110 pwm mode 1 OC4PE = 1 OC4FE = 0 CC4S[1:0] = 0b00 OUT
    TIM1->CCR1 = tBldc.Duty;
    TIM1->CCR2 = tBldc.Duty;
    TIM1->CCR3 = tBldc.Duty;
}
/***********************************************************************
* 函数名称  : Bldc_Pwm_On_CB
* 功能描述  ：C(W)相绕组跟B(V)相绕组通电
* 形参      ：无
* 返回值    ：无
* 要点备注  ：电流从C相流进，从B相流出
***********************************************************************/
static void Bldc_Pwm_On_CB(void)
{
    TIM1->CCER = 0x00001140; //CC3E = 1  CC2NE = 1 CC4E = 1
    TIM1->CCMR1 = 0x00005800;//OC2CE = 0 OC2M[2:0] = 0b101 Force active level OC2PE = 1 OC2FE = 0 CC2S[1:0] = 0b00 OUT
                             //OC1CE = 0 OC1M[2:0] = 0b000 Frozen OC1PE = 0 OC1FE = 0 CC1S[1:0] = 0b00 OUT
    TIM1->CCMR2 = 0x00006868; //OC3CE = 0 OC3M[2:0] = 0b110 pwm mode 1 OC3PE = 1 OC3FE = 0 CC3S[1:0] = 0b00 OUT
                              //OC4CE = 0 OC4M[2:0] = 0b110 pwm mode 1 OC4PE = 1 OC4FE = 0 CC4S[1:0] = 0b00 OUT
    TIM1->CCR1 = tBldc.Duty;
    TIM1->CCR2 = tBldc.Duty;
    TIM1->CCR3 = tBldc.Duty;
}
/***********************************************************************
* 函数名称  : Bldc_Pwm_Off
* 功能描述  ：所有PWM信号禁止输出
* 形参      ：无
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
static void Bldc_Pwm_Off(void)
{
    TIM1->CCER = 0x00001ddd; //CC1E = 1  CC1NE = 1  CC1NP = 1 CC2E = 1  CC2NE = 1  CC2NP = 1 CC3E = 1  CC3NE = 1  CC3NP = 1 CC4E = 1
    TIM1->CCMR1 = 0x00004848;//OC2CE = 0 OC2M[2:0] = 0b100 Force inactive level OC2PE = 1 OC2FE = 0 CC2S[1:0] = 0b00 OUT
                             //OC1CE = 0 OC1M[2:0] = 0b100 Force inactive level OC1PE = 1 OC1FE = 0 CC1S[1:0] = 0b00 OUT
    TIM1->CCMR2 = 0x00006848; //OC3CE = 0 OC3M[2:0] = 0b100 Force inactive level OC3PE = 1 OC3FE = 0 CC3S[1:0] = 0b00 OUT
                              //OC4CE = 0 OC4M[2:0] = 0b110 pwm mode 1 OC4PE = 1 OC4FE = 0 CC4S[1:0] = 0b00 OUT
    TIM1->CCR1 = 0;
    TIM1->CCR2 = 0;
    TIM1->CCR3 = 0;
}
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
