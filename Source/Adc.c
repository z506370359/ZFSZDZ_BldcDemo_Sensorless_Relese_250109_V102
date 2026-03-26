/***********************************************************************
* @file           Adc.c
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    模拟量采集处理模块
***********************************************************************/
#include "../Include/Adc.h"

/***********************************************************************
* 函数名称  : AdcSample
* 功能描述  ：获取ADC转换完成数据
* 形参      ：无
* 返回值    ：无
* 要点备注  ：DMA搬运完数据，读取DMA结构体中的数据
***********************************************************************/
void AdcSample(void)
{
    tAdc.An0.ValueInstant = tDMA.ADDATA0; //UA/UB/UC 端电压采样转换值
    tAdc.An3.ValueInstant = tDMA.ADDATA1; //VBUS 采样转换值
    tAdc.An4.ValueInstant = tDMA.ADDATA2; //IBUS 采样转换值
    tAdc.An5.ValueInstant = tDMA.ADDATA3; //POT 旋钮采样转换值
    tAdc.An9.ValueInstant = tDMA.ADDATA4; //NTC 采样转换值
    
    tAdc.TerminalVoltage = tAdc.An0.ValueInstant;
    tAdc.Vbus = tAdc.An3.ValueInstant;
}
/***********************************************************************
* 函数名称  : AdcFilter
* 功能描述  ：ADC转换结果平均值滤波处理
* 形参      ：无
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void AdcFilter(void)
{
    //VBUS 平均值滤波处理
    tAdc.An3.Sum += tAdc.An3.ValueInstant;
    tAdc.An3.Num ++;
    if(tAdc.An3.Num >= 128)
    {
        tAdc.An3.ValueAverage = (unsigned short)(tAdc.An3.Sum >> 7);
        tAdc.An3.Num = 0;
        tAdc.An3.Sum = 0;
    }
    //IBUS 平均值滤波处理
    tAdc.An4.Sum += tAdc.An4.ValueInstant;
    tAdc.An4.Num ++;
    if(tAdc.An4.Num >= 128)
    {
        tAdc.An4.ValueAverage = (unsigned short)(tAdc.An4.Sum >> 7);
        tAdc.An4.Num = 0;
        tAdc.An4.Sum = 0;
    }
    //POT 平均值滤波处理
    tAdc.An5.Sum += tAdc.An5.ValueInstant;
    tAdc.An5.Num ++;
    if(tAdc.An5.Num >= 128)
    {
        tAdc.An5.ValueAverage = (unsigned short)(tAdc.An5.Sum >> 7);
        tAdc.An5.Num = 0;
        tAdc.An5.Sum = 0;
    }
    //NTC 平均值滤波处理
    tAdc.An9.Sum += tAdc.An9.ValueInstant;
    tAdc.An9.Num ++;
    if(tAdc.An9.Num >= 128)
    {
        tAdc.An9.ValueAverage = (unsigned short)(tAdc.An9.Sum >> 7);
        tAdc.An9.Num = 0;
        tAdc.An9.Sum = 0;
    }
}
/***********************************************************************
* 函数名称  : AdcSetSpeed
* 功能描述  ：根据旋钮转换值处理为占空比调节范围值或速度闭环目标值
* 形参      ：无
* 返回值    ：无
* 要点备注  ：开环时使用tAdc.SetDuty  速度闭环时使用 tAdc.SetSpeed
***********************************************************************/
static void AdcSetSpeed(void)
{
    unsigned short Temp;
    unsigned short TempA;
    
    Temp = tAdc.An5.ValueAverage >> 1;
    if(Temp > 2000)
        Temp = 2000;
    tAdc.SetDuty = Temp; //0~4095 处理为 0~2000
    
    TempA = tAdc.An5.ValueAverage << 4; //0~65520
    TempA = (TempA * 3100) >> 16;
    if(TempA > 3000)
        TempA = 3000;
    if(TempA < 100)
        TempA = 0;
    tAdc.SetSpeed = TempA; //0~4095 处理为 0~3000
}
/***********************************************************************
* 函数名称  : GetVbusValue
* 功能描述  ：根据AD值获取Vbus电压真实值
* 形参      ：无
* 返回值    ：无
* 要点备注  ：10*Vbus+ = 5280 * Vbus(AD) >> 16
***********************************************************************/
static void GetVbusValue(void)
{
    tAdc.VbusReal = tAdc.An3.ValueAverage * VBUS_FACTOR >> 16;
}
/***********************************************************************
* 函数名称  : GetTemperatureValue
* 功能描述  ：查表获取温度值
* 形参      ：TadValue 温度对应的AD值
* 返回值    ：实际温度值
* 要点备注  ：无
***********************************************************************/
static char GetTemperatureValue(unsigned short TadValue)
{
    signed char Temperature;
    static unsigned short Index = 70;
    
    if(TadValue > TempTable[0]) //温度低于-40°
    {
        Temperature = MIN_TEMP - 1;
        Index = 0;
    }
    else if(TadValue <= TempTable[NTC_POINT - 1]) //温度大于等于125°时
    {
        Temperature = MAX_TEMP;
        Index = NTC_POINT - 1;
    }
    else //温度在 -40°~125°
    {
        if((TadValue <= TempTable[Index]) && (TadValue > TempTable[Index+1])) //温度在30°
        {
            Temperature = MIN_TEMP + Index;
        }
        else if(TadValue > TempTable[Index]) //温度低于30°
        {
            for(;Index>0;Index--)
            {
                if(TadValue <= TempTable[Index]) //实际的AD值小于等于查表值，找到了温度对应的索引号
                {
                    Temperature = MIN_TEMP + Index;
                    break;
                }
            }
        }
        else //温度在30°以上
        {
            Index += 1;
            for(;Index< NTC_POINT - 1;Index++)
            {
                if(TadValue > TempTable[Index + 1])
                {
                    Temperature = MIN_TEMP + Index;
                    break;
                }
            }
        }
    }
    return (Temperature);
}
/***********************************************************************
* 函数名称  : AdcApp
* 功能描述  ：ADC模块应用程序
* 形参      ：无
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void AdcApp(void)
{
    AdcSetSpeed();
    GetVbusValue(); //获取Vbus真实值
    tAdc.NtcReal = GetTemperatureValue(tAdc.An9.ValueAverage);//获取温度真实值
}
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
