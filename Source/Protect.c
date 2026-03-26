/***********************************************************************
* @file           Protect.c
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    故障监测及处理模块
***********************************************************************/
#include "../Include/Protect.h"

/***********************************************************************
* 函数名称  : HardOverCurrentMonitor
* 功能描述  ：硬件过流监测
* 形参      ：无
* 返回值    ：无
* 要点备注  ：14A安培触发过流保护
***********************************************************************/
void HardOverCurrentMonitor(void)
{
    if(TIM1->SR & 0x00000080) //查询BIF刹车标志位，判断是否有故障发生
    {
        TIM1->SR &= 0xffffff7f;
        tProtect.HardOverCurrentCount ++;
        if(tProtect.HardOverCurrentCount >= 5)
        {
            tProtect.HardOverCurrentCount = 0;
            tProtect.Code = 1;//设置故障代码
            //关闭PWM输出
            TIM1->CCER = 0x00001ddd;
            TIM1->CCMR1 = 0x00004848;
            TIM1->CCMR2 = 0x00006848;
            
            TIM1->CCR1 = 0;
            TIM1->CCR2 = 0;
            TIM1->CCR3 = 0;
        }
    }
    else
    {
        tProtect.HardOverCurrentCount = 0;
    }
}
/***********************************************************************
* 函数名称  : ProtectInit
* 功能描述  ：故障监测初始化程序
* 形参      ：无
* 返回值    ：无
* 要点备注  ：对计数值做初值设置
***********************************************************************/
void ProtectInit(void)
{
    tProtect.SoftOverCurrentCount = 100;
    tProtect.OverVoltageCount = 100;
    tProtect.UnderVoltageCount = 100;
    tProtect.OverTemperatureCount = 100;
}
/***********************************************************************
* 函数名称  : ProtectApp
* 功能描述  ：故障监测应用程序
* 形参      ：无
* 返回值    ：无
* 要点备注  ：软件过流、Vbus过欠压、过温
***********************************************************************/
void ProtectApp(void)
{
    if(tProtect.SoftOverCurrentCount)
        tProtect.SoftOverCurrentCount --;
    
    if(tProtect.OverVoltageCount)
        tProtect.OverVoltageCount --;
    
    if(tProtect.UnderVoltageCount)
        tProtect.UnderVoltageCount --;
    
    if(tProtect.OverTemperatureCount)
        tProtect.OverTemperatureCount --;
    
    //软件过流保护 检测到母线瞬时电流大于4A，持续时间100ms
    if(tAdc.An4.ValueAverage > SOCTHRESHOLD)
    {
        if(tProtect.SoftOverCurrentCount == 0)
        {
            if((tProtect.Code == 0) || (tProtect.Code > 3))
            {
                tProtect.Code = 4;
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
    else
    {
        tProtect.SoftOverCurrentCount = 100; //100ms
    }
    //Vbus过欠压保护
    if(tAdc.VbusReal > OVTHRESHOLD) //过压
    {
        if(tProtect.OverVoltageCount == 0)
        {
            if((tProtect.Code == 0) || (tProtect.Code > 4))
            {
                tProtect.Code = 5;
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
    else if(tAdc.VbusReal < LVTHRESHOLD) //欠压
    {
        if(tProtect.UnderVoltageCount == 0)
        {
            if((tProtect.Code == 0) || (tProtect.Code > 5))
            {
                tProtect.Code = 6;
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
    else if((tAdc.VbusReal >= LVRETHRESHOLD) && (tAdc.VbusReal <= OVRETHRESHOLD)) //过欠压恢复
    {
        if((tProtect.Code == 5) || (tProtect.Code == 6))
        {
            tProtect.Code = 0;
        }
        tProtect.UnderVoltageCount = 100;
        tProtect.OverVoltageCount = 100;
    }
    else
    {
        tProtect.UnderVoltageCount = 100;
        tProtect.OverVoltageCount = 100;
    }
    //过温检测
    if(tAdc.NtcReal > OTTHRESHOLD) //过温判断
    {
        if(tProtect.OverTemperatureCount == 0)
        {
            if((tProtect.Code == 0) || (tProtect.Code > 6))
            {
                tProtect.Code = 7;
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
    else if(tAdc.NtcReal <= OTRETHRESHOLD) //过温恢复判断
    {
        if(tProtect.Code == 7)
            tProtect.Code = 0;
        tProtect.OverTemperatureCount = 100;
    }
    else
    {
        tProtect.OverTemperatureCount = 100;
    }
}

/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
