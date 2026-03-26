/***********************************************************************
* @file           Main.c
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    张飞实战电子"扬帆起航"课程无感方波控制部分
***********************************************************************/
#include "../Include/Main.h"

/***********************************************************************
* 函数名称  ：MainLoop
* 功能描述  ：主函数
* 形参      ：无
* 返回值    ：无
* 要点备注  ：单片机上电复位向量执行后跳转到该函数
***********************************************************************/
void MainLoop(void)
{    
    Initialize();
    tSystem.StartDelayCount = 500; //设置上电延时变量初值
    SysTick->CSR |= 0x00000001; //ENABLE = 1 使能滴答定时
    NVIC->ISER[0] = 0x00010200; //使能DMA通道一中断，使能定时器3中断
    
    while(tSystem.StartDelayCount != 0); //等待上电延时结束
    
    do
    {
        if(tSystem.LcdCount == 0) //500ms刷新一次
        {
            tSystem.LcdCount = 500;
            Display();
        }
    }while(1);
}
/***********************************************************************
* 函数名称  : DMA1_Channel1_IRQHandler
* 功能描述  ：DMA通道一中断服务函数
* 形参      ：无
* 返回值    ：无
* 要点备注  ：DMA搬运ADC数据，搬运完成后，进入中断  16KHz 62.5us
***********************************************************************/
void DMA1_Channel1_IRQHandler(void)
{
    unsigned int Temp;
    
    Temp = DMA1->ISR;
    if(Temp & 0x00000002)
    {
        //TEST_PIN_LOW;
        DMA1->IFCR |= 0x00000002; //清除中断标志位
        AdcSample(); //读取ADC转换完成结果
        BldcStateMachine(); //BLDC电机运行状态机
        HardOverCurrentMonitor(); //硬件过流监测
        SentReceData(); //串口发送接收数据
        UsartDmaRestart();//DMA重启，发送数据到上位机
        AdcFilter(); //ADC瞬时值平均值滤波求取
        //TEST_PIN_HIGH;
    }
}
/***********************************************************************
* 函数名称  : TIM3_IRQHandler
* 功能描述  ：定时器3换相中断
* 形参      ：无
* 返回值    ：无
* 要点备注  ：根据定时设置时间进入改中断更换绕组通电相序
***********************************************************************/
void TIM3_IRQHandler(void)
{
    unsigned int Temp;
    
    Temp = TIM3->SR;
    if(Temp & 0x0001)
    {
        TIM3->SR &= 0xfffe; //UIF = 0 清除定时器3更新中断标志位
        
        tBldc.PhaseStep = GetPhaseValue(tBldc.PhaseStep,tMotor.Direction);
        tBldc.Slope = GetSlope(tBldc.PhaseStep,tMotor.Direction);
        Bldc_SetPwm_OnOff(tBldc.PhaseStep); //给绕组通电
        AdcChannelSwitch(tBldc.PhaseStep); //端电压AD采样通道切换
        tBldc.DemagnetizationTime = 8;//转速高 电流大，设置退磁时间长 
                                     //转速低 电流小，设置退磁时间短
        tBldc.SensorlessState = 1; //等待退磁结束
        
        tProtect.LockRotorCurrentCount = 480;//30ms
        tProtect.LockRotorPhaseCount = 1600;//100ms
        
        TIM3->CR1 &= 0xfffe; //CEN = 0 关闭定时3
        TIM3->CNT = 0;
    }
}
/***********************************************************************
* 函数名称  : SysTick_Handler
* 功能描述  ：滴答定时器中断服务函数
* 形参      ：无
* 返回值    ：无
* 要点备注  ：1ms进入该中断一次
***********************************************************************/
void SysTick_Handler(void)
{
    unsigned int Temp;
    
    Temp = SysTick->CSR;
    
    if(Temp & 0x00010000) //判断COUNTFLAG = 1
    {
        if(tSystem.StartDelayCount)
            tSystem.StartDelayCount --;  //上电延时变量 1ms递减 1
        
        if(tSystem.LcdCount)
            tSystem.LcdCount --; //LCD刷新变量 1ms递减 1
        
        if(tSystem.SpeedAdjustCount)
            tSystem.SpeedAdjustCount --; //斜坡更新变量 1ms递减1
        
        if(tSystem.UsartCount) //串口通讯计数变量
            tSystem.UsartCount --;
        
        if(tSystem.DutyCount) //开环占空比斜坡变化计数变量
            tSystem.DutyCount --;
        
        if(tSystem.StartDelayCount == 0) //上电延时结束后
        {
            AdcApp(); //ADC模块应用程序
            MotorApp(); //电机运行模块应用程序
            UsartApp(); //串口通讯模块应用程序
           // ProtectApp();//故障监测及处理应用程序
        }
    }
}
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
