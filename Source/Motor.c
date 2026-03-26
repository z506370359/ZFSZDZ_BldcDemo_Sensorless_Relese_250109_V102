/***********************************************************************
* @file           Motor.c
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    电机运行控制应用模块
***********************************************************************/
#include "../Include/Motor.h"

/***********************************************************************
* 函数名称  : MotorInit
* 功能描述  ：电机运行模块初始化函数
* 形参      ：无
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void MotorInit(void)
{
    tMotor.SpeedMode = DEFAULT_SPEEDMODE; //默认调速方式：旋钮
    tMotor.RunMode = DEFAULT_RUNMODE; //默认运行模式：闭环运行
    tMotor.Direction = DEFAULT_DIR; //默认旋转方向：逆时针
    PIInit(&tPI_Speed,16384,128,31129,32767,0);
    tMotor.VarDelta = 80; //设置斜坡增量初始值
    tMotor.AimDuty = RUN_MIN_DUTY; //设置目标占空比初始值
}
/***********************************************************************
* 函数名称  : MotorApp
* 功能描述  ：电机运行模块应用程序
* 形参      ：无
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
void MotorApp(void)
{
    MotorRunStopControl(); //电机的启停控制
    
    if(tMotor.SpeedMode == SPEEDMODE_POT) //电位器调速
    {
        if(tMotor.RunMode == RUNMODE_OPENLOOP)
        {
            MotorSetDuty(tAdc.SetDuty);
        }
        else if(tMotor.RunMode == RUNMODE_CLOSELOOP)
        {
            MotorSetSpeed(tAdc.SetSpeed);
        }
    }
    else if(tMotor.SpeedMode == SPEEDMODE_USART) //串口调速
    {
        if(tMotor.RunMode == RUNMODE_OPENLOOP)
        {
            MotorSetDuty(tUsart.Duty);
        }
        else if(tMotor.RunMode == RUNMODE_CLOSELOOP)
        {
            MotorSetSpeed(tUsart.SetSpeed);
        }
    }
    //MotorSetSpeed(tAdc.SetSpeed); //获取速度闭环目标转速
    
    CalcMotorSpeed(); //计算电机转速
    MotorSpeedControl();//速度闭环控制
    TIM16->CCR1 = tMotor.ActSpeed;
    TIM17->CCR1 = tMotor.AimSpeed;
}
/***********************************************************************
* 函数名称  : MotorRunStopControl
* 功能描述  ：电机运行停止控制
* 形参      ：无
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
static void MotorRunStopControl(void)
{
    if(tMotor.RunStopState == 0) //在电机停止运行状态下，判断是否要启动
    {
        if(tMotor.SpeedMode == SPEEDMODE_POT) //电位器调速及启动
        {
            if(tMotor.RunMode == RUNMODE_OPENLOOP)
            {
                if((tAdc.SetDuty > 50) && (tProtect.Code == 0))
                {
                    tMotor.RunStopState = 1;
                    tBldc.State = 1;
                }
            }
            else if(tMotor.RunMode ==  RUNMODE_CLOSELOOP)
            {
                if((tAdc.SetSpeed > 80) && (tProtect.Code == 0))
                {
                    tMotor.RunStopState = 1;
                    tBldc.State = 1;
                }
            }
        }
        else if(tMotor.SpeedMode == SPEEDMODE_USART) //串口数据调速及启动
        {
            if(tMotor.RunMode == RUNMODE_OPENLOOP)
            {
                if((tUsart.Duty > 50) && (tProtect.Code == 0))
                {
                    tMotor.RunStopState = 1;
                    tBldc.State = 1;
                }
            }
            else if(tMotor.RunMode ==  RUNMODE_CLOSELOOP)
            {
                if((tUsart.SetSpeed > 0) && (tProtect.Code == 0))
                {
                    tMotor.RunStopState = 1;
                    tBldc.State = 1;
                }
            }
        }
    }
    else //在电机运行状态下，判断是否要停机
    {
        if(tMotor.SpeedMode == SPEEDMODE_POT) //通过电位器设定停机条件
        {
            if(tMotor.RunMode == RUNMODE_OPENLOOP)
            {
                if((tAdc.SetDuty < 40) || (tProtect.Code != 0))
                {
                    tMotor.RunStopState = 0;
                    tBldc.State = 0;
                }
            }
            else if(tMotor.RunMode ==  RUNMODE_CLOSELOOP)
            {
                if((tAdc.SetSpeed < 80) || (tProtect.Code != 0))
                {
                    tMotor.RunStopState = 0;
                    tBldc.State = 0;
                }
            }
        }
        else if(tMotor.SpeedMode == SPEEDMODE_USART) //通过串口数据设定停机条件
        {
            if(tMotor.RunMode == RUNMODE_OPENLOOP)
            {
                if((tUsart.Duty < 40) || (tProtect.Code != 0))
                {
                    tMotor.RunStopState = 0;
                    tBldc.State = 0;
                }
            }
            else if(tMotor.RunMode ==  RUNMODE_CLOSELOOP)
            {
                if((tUsart.SetSpeed == 0) || (tProtect.Code != 0))
                {
                    tMotor.RunStopState = 0;
                    tBldc.State = 0;
                }
            }
        }
    }
}
/***********************************************************************
* 函数名称  : CalcMotorSpeed
* 功能描述  ：计算电机转速
* 形参      ：无
* 返回值    ：无
* 要点备注  ：Ne = 60*f/Np = 60/(T*Np) = 60/(t*Nn*Np) 
                 = 60/(0.000002*Nn*Np) = 30000000 / (MperiodCount)
***********************************************************************/
static void CalcMotorSpeed(void)
{
    unsigned char i;
    unsigned int SumZeventTime;
    unsigned int EperiodCount;
    unsigned int MperiodCount;
    
    if(tBldc.SpeedUpdate)
    {
        tBldc.SpeedUpdate = 0;
        SumZeventTime = 0;
        i = 0;
        
        do
        {
            SumZeventTime += tBldc.ZeventTimeArray[i++];
        }while(i <  6);
        EperiodCount = SumZeventTime; //Nn
        MperiodCount = EperiodCount * 2; //Nn*Np
        
        tMotor.InstantSpeed = (unsigned short)(30000000 / MperiodCount); //Ne = 30000000 / (MperiodCount)
        tMotor.ActSpeed = tMotor.InstantSpeed;
        tMotor.DisPlaySpeed = LowPassFilter(FACTOR_K,tMotor.InstantSpeed,tMotor.DisPlaySpeed); //速度低通滤波
    }
}
/***********************************************************************
* 函数名称  : MotorSetDuty
* 功能描述  ：设置PWM占空比
* 形参      ：Var:目标值设定量 （旋钮、串口）
* 返回值    ：处理之后的目标值
* 要点备注  ：设定值的斜坡处理
***********************************************************************/
static void MotorSetDuty(unsigned short Var)
{
    if(tSystem.DutyCount == 0)
    {
        tSystem.DutyCount = 100; //100ms占空比变化一次
        tMotor.AimDuty = GetAimValue(Var,tMotor.AimDuty,50,RUN_MIN_DUTY);
    }
}
/***********************************************************************
* 函数名称  : MotorSetSpeed
* 功能描述  ：电机转速闭环控制目标转速给定
* 形参      ：Var：目标转速设置值(电位器采样转换值、串口上位机)
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
static void MotorSetSpeed(unsigned short Var)
{
    if(tSystem.SpeedAdjustCount == 0)
    {
        tSystem.SpeedAdjustCount = 100;
        
        if(tBldc.SpeedEnable)
            tMotor.AimSpeed = GetAimValue(Var,tMotor.AimSpeed,RUN_MIN_SPEED,100);
        else
            tMotor.AimSpeed = 0;
    }
}
/***********************************************************************
* 函数名称  : GetAimValue
* 功能描述  ：根据串口或者电位器设置值，按照设定的斜坡得到目标值
* 形参      ：Var：目标转速设置值(电位器采样转换值、串口上位机)
              Aim: 斜坡处理后的值
              MinValue：目标值的最小值设定
              MaxDelta：增量或者减量，每一步的变化量
* 返回值    ：Aim 经过斜坡处理之后的目标值
* 要点备注  ：无
***********************************************************************/
static unsigned short GetAimValue(unsigned short Var,unsigned short Aim,unsigned short MinValue,unsigned short MaxDelta)
{
    unsigned short Temp;
    
    if(Var < Aim) //下坡阶段
    {
        Temp = Aim - Var; //求取当前目标值跟设定值的差值
        if(Temp > tMotor.VarDelta) //判断差值跟增量/减量的大小情况
        {
            Temp = Aim - tMotor.VarDelta;
        }
        else
        {
            Temp = Var;
        }
        if(Temp < MinValue) //最小值限制
            Temp = MinValue;
        Aim = Temp;
    }
    else if(Var > Aim) //上坡阶段
    {
        Temp = Var - Aim; //求取当前目标值跟设定值的差值
        if(Temp > tMotor.VarDelta)
        {
            Temp = Aim + tMotor.VarDelta;
        }
        else
        {
            Temp = Var;
        }
        if(Temp < MinValue) //最小值限制
            Temp = MinValue;
        Aim = Temp;
    }
    if(tMotor.VarDelta < MaxDelta)
        tMotor.VarDelta ++;
    return (Aim);
}
/***********************************************************************
* 函数名称  : MotorSpeedControl
* 功能描述  ：使用PI控制器进行速度闭环控制
* 形参      ：无
* 返回值    ：无
* 要点备注  ：无
***********************************************************************/
static void MotorSpeedControl(void)
{
    signed short qTemp;
    
    if(tBldc.SpeedEnable)
    {
        if(tMotor.RunMode == RUNMODE_CLOSELOOP)
        {
            tPI_Speed.qInRef = tMotor.AimSpeed * 10; //0~3000 ---> 0~30000
            tPI_Speed.qInFeb = tMotor.ActSpeed * 10; //0~3000 ---> 0~30000
            PIControl(&tPI_Speed); //速度PI闭环控制
            qTemp = tPI_Speed.qOut * (PWMARRVALUE >> 1) >> 15; //0~32767 ---> 0~2000
            if(qTemp < RUN_MIN_DUTY)
                qTemp = RUN_MIN_DUTY;
            tBldc.AimDuty = (unsigned short)qTemp;
        }
        else if(tMotor.RunMode == RUNMODE_OPENLOOP)
        {
            tBldc.AimDuty = tMotor.AimDuty;
        }
    }
    else
    {
        tBldc.AimDuty = RUN_MIN_DUTY;
        tPI_Speed.qSum = 0;
        tPI_Speed.qOut = 0;
    }
}
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
