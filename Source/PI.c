/***********************************************************************
* @file           PI.c
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    PI控制器计算模块
***********************************************************************/
#include "../Include/PI.h"

/***********************************************************************
* 函数名称  : PIInit
* 功能描述  ：pi控制器参数初始化
* 形参      ：PIStrcut PI控制器结构体指针 Kp比例项系数 Ki积分项系数
              Kc 抗积分饱和系数 Max限幅最大值 Min限幅最小值
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void PIInit(tPI_Type *PIStrcut,signed short Kp,signed short Ki,signed short Kc,signed short Max,signed short Min)
{
   PIStrcut->qKp = Kp;
   PIStrcut->qKi = Ki;
   PIStrcut->qKc = Kc;
   PIStrcut->qOutMax = Max;
   PIStrcut->qOutMin = Min;
   PIStrcut->qOut = 0;
   PIStrcut->qSum = 0;     
}
/***********************************************************************
* 函数名称  : PIControl
* 功能描述  ：PI控制器模块
* 形参      ：PIStrcut PI控制器结构体指针
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void PIControl(tPI_Type *PIStrcut)
{
    signed short Error;
    signed int U;
    signed short OutTemp;
    
    Error = PIStrcut->qInRef - PIStrcut->qInFeb;
    U = PIStrcut->qKp * Error; //Q30
    U += PIStrcut->qSum;
    OutTemp = (signed short)(U >> 15); //Q15
    
    //限幅处理
    if(OutTemp > PIStrcut->qOutMax)
        PIStrcut->qOut = PIStrcut->qOutMax;
    else if(OutTemp < PIStrcut->qOutMin)
        PIStrcut->qOut = PIStrcut->qOutMin;
    else
        PIStrcut->qOut = OutTemp;
    
    U = Error * PIStrcut->qKi; 
    Error = OutTemp - PIStrcut->qOut;
    U -= Error * PIStrcut->qKc; //抗积分饱和处理
    PIStrcut->qSum = U + PIStrcut->qSum; //抗积分饱和处理之后的累加项
}
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
