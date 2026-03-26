/***********************************************************************
* @file           IPD.c
* @author         ZhaoYun(ZFSZDZ)
* @date           2025.01.10
* @Version        V1.0.0
* @description    转子初始位置检测
***********************************************************************/
#include "../Include/IPD.h"

/***********************************************************************
* 函数名称  : IPDInit
* 功能描述  ：初始位置检测模块初始化
* 形参      ：无
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void IPDInit(void)
{
    tIPD.Count = 8;
    tIPD.Step = 0;
    tIPD.RotorSector = 0;
}
/***********************************************************************
* 函数名称  : Init_Position_Detect
* 功能描述  ：初始位置检测步骤实现
* 形参      ：无
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void Init_Position_Detect(void)
{
    if(tIPD.Count)
        tIPD.Count --;
    
    switch(tIPD.Step)
    {
        case 0: //关闭所有管子
            TIM1->CCER = 0x00001ddd; 
            TIM1->CCMR1 = 0x00004848;
            TIM1->CCMR2 = 0x00006848;
            
            tIPD.Step ++;
            break;
        case 1: //A+B-
            TIM1->CCER = 0x00001d41; 
            TIM1->CCMR1 = 0x00005858;
            TIM1->CCMR2 = 0x00006848; 
            
            tIPD.Step ++;
            tIPD.Count = 8;
            break;
        case 2: //等待A+B-设置通电时间结束
            tIPD.CurrentValue[0] = tAdc.An4.ValueInstant;
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x00001ddd; 
                TIM1->CCMR1 = 0x00004848;
                TIM1->CCMR2 = 0x00006848;
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 3: //B+A-
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x00001d14; 
                TIM1->CCMR1 = 0x00005858;
                TIM1->CCMR2 = 0x00006848; 
                
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 4://关闭所有驱动
            tIPD.CurrentValue[1] = tAdc.An4.ValueInstant;
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x00001ddd; 
                TIM1->CCMR1 = 0x00004848;
                TIM1->CCMR2 = 0x00006848;
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 5: //A+C-
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x000014d1; 
                TIM1->CCMR1 = 0x00004858;
                TIM1->CCMR2 = 0x00006858; 
                
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 6: //关闭所有驱动
            tIPD.CurrentValue[2] = tAdc.An4.ValueInstant;
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x00001ddd; 
                TIM1->CCMR1 = 0x00004848;
                TIM1->CCMR2 = 0x00006848;
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 7: //C+A-
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x000011d4; 
                TIM1->CCMR1 = 0x00004858;
                TIM1->CCMR2 = 0x00006858; 
                
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 8: //关闭所有驱动
            tIPD.CurrentValue[3] = tAdc.An4.ValueInstant;
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x00001ddd; 
                TIM1->CCMR1 = 0x00004848;
                TIM1->CCMR2 = 0x00006848;
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 9: //B+C-
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x0000141d; 
                TIM1->CCMR1 = 0x00005848;
                TIM1->CCMR2 = 0x00006858; 
                
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 10: //关闭所有驱动
            tIPD.CurrentValue[4] = tAdc.An4.ValueInstant;
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x00001ddd; 
                TIM1->CCMR1 = 0x00004848;
                TIM1->CCMR2 = 0x00006848;
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 11: //C+B-
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x0000114d; 
                TIM1->CCMR1 = 0x00005848;
                TIM1->CCMR2 = 0x00006858; 
                
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 12: //关闭所有驱动
            tIPD.CurrentValue[5] = tAdc.An4.ValueInstant;
            if(tIPD.Count == 0)
            {
                TIM1->CCER = 0x00001ddd; 
                TIM1->CCMR1 = 0x00004848;
                TIM1->CCMR2 = 0x00006848;
                tIPD.Step ++;
                tIPD.Count = 8;
            }
            break;
        case 13: //电流和的计算
            if(tIPD.Count == 0)
            {
                tIPD.SumIab_Iba = tIPD.CurrentValue[0] + tIPD.CurrentValue[1];
                tIPD.SumIac_Ica = tIPD.CurrentValue[2] + tIPD.CurrentValue[3];
                tIPD.SumIbc_Icb = tIPD.CurrentValue[4] + tIPD.CurrentValue[5];
                
                tIPD.Step ++;
            }
            break;
        case 14: //根据电流和判断转子扇区
            if((tIPD.SumIab_Iba < tIPD.SumIac_Ica) && (tIPD.SumIab_Iba < tIPD.SumIbc_Icb))
            {
                if((tIPD.CurrentValue[2] + tIPD.CurrentValue[4]) >= (tIPD.CurrentValue[3] + tIPD.CurrentValue[5]))
                {
                    tIPD.RotorSector = 6;
                }
                else
                {
                    tIPD.RotorSector = 3;
                }
            }
            else if((tIPD.SumIac_Ica < tIPD.SumIab_Iba) && (tIPD.SumIac_Ica < tIPD.SumIbc_Icb))
            {
                if((tIPD.CurrentValue[0] + tIPD.CurrentValue[5]) >= (tIPD.CurrentValue[4] + tIPD.CurrentValue[1]))
                {
                    tIPD.RotorSector = 4;
                }
                else
                {
                    tIPD.RotorSector = 1;
                }
            }
            else if((tIPD.SumIbc_Icb < tIPD.SumIab_Iba) && (tIPD.SumIbc_Icb < tIPD.SumIac_Ica))
            {
                if((tIPD.CurrentValue[0] + tIPD.CurrentValue[2]) >= (tIPD.CurrentValue[1] + tIPD.CurrentValue[3]))
                {
                    tIPD.RotorSector = 5;
                }
                else
                {
                    tIPD.RotorSector = 2;
                }
            }
            tIPD.Step ++;
            break;
        case 15:
            if(tMotor.Direction == DIR_CW)
            {
                if(tIPD.RotorSector == 5)
                {
                    tBldc.PhaseStep = 1;
                }
                else if(tIPD.RotorSector == 4)
                {
                    tBldc.PhaseStep = 6;
                }
                else if(tIPD.RotorSector == 3)
                {
                    tBldc.PhaseStep = 5;
                }
                else if(tIPD.RotorSector == 2)
                {
                    tBldc.PhaseStep = 4;
                }
                else if(tIPD.RotorSector == 1)
                {
                    tBldc.PhaseStep = 3;
                }
                else if(tIPD.RotorSector == 6)
                {
                    tBldc.PhaseStep = 2;
                }
                else
                {
                    tIPD.Step = 0;
                }
            }
            else
            {
                if(tIPD.RotorSector == 5)
                {
                    tBldc.PhaseStep = 2;
                }
                else if(tIPD.RotorSector == 4)
                {
                    tBldc.PhaseStep = 1;
                }
                else if(tIPD.RotorSector == 3)
                {
                    tBldc.PhaseStep = 6;
                }
                else if(tIPD.RotorSector == 2)
                {
                    tBldc.PhaseStep = 5;
                }
                else if(tIPD.RotorSector == 1)
                {
                    tBldc.PhaseStep = 4;
                }
                else if(tIPD.RotorSector == 6)
                {
                    tBldc.PhaseStep = 3;
                }
                else
                {
                    tIPD.Step = 0;
                }
            }
            if(tIPD.RotorSector != 0)
                tBldc.State = 2;
            break;
        default:
            break;
    }
}
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
