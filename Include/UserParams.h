/***********************************************************************
* @file           UserParams.h
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    用户自定义宏定义
***********************************************************************/
#define     SYSTEMFREQ              (64000000) //单片机主频 64MHz
#define     PWMFREQ                 (16000) //16KHz
#define     PWMARRVALUE             (SYSTEMFREQ / PWMFREQ) //PWM模块重载值

#define     ORIENT_DUTY             (200)   //5%~10% 预定位占空比设置
#define     ORIENT_TIME             (160)  //10ms 定位时间设置

#define     SPEEDMODE_POT           (0x00)
#define     SPEEDMODE_USART         (0x01)
#define     DEFAULT_SPEEDMODE       (SPEEDMODE_POT) //默认使用电位器启动机调速

#define     RUNMODE_OPENLOOP        (0x00)
#define     RUNMODE_CLOSELOOP       (0x01)
#define     DEFAULT_RUNMODE         (RUNMODE_CLOSELOOP) //默认闭环工作

#define     DIR_CW                  (0)
#define     DIR_CCW                 (1)
#define     DEFAULT_DIR             (DIR_CCW) //默认逆时针转动
            
#define     DUTY_MAX                (500) //25%
#define     DUTY_MIN                (300) //15%
#define     DUTY_STEP               (1)

//RPM = 60*f/P = 60/T360*P = 60/T60*6*P = 60 000000/T60*62.5us*6*2 = 80000/T60(count)
#define     FORCEPHASETIME_MAX      (266) //300RPM
#define     FORCEPHASETIME_MIN      (133) //600RPM
#define     FORCEPHASETIME_STEP     (10)

#define     BEMF_ZERO_FACTOR        (30000) //理论值为50% 实际情况可以灵活调整
#define     PHASE_N_FACTOR          (5242) //升频升压阶段 换相系数N  Vbus*N%

#define     RUN_MIN_SPEED           (350)
#define     RUN_MAX_SPEED           (3000)

#define     RUN_MIN_DUTY            (200) //10%
#define     RUN_MAX_DUTY            (2000) //1000%

#define     FACTOR_K                (20578) //K = Ts*2π*fc = 0.001*6.28*50Hz = 0.314 * 65536 = 20578

#define     VBUS_FACTOR             (5280) //Vbus+ = 20K / 2K * Vbus(AD) * 3.3V / 4096 = Vbus+ = 0.008056640625 * Vbus(AD) 
                                           //10 * Vbus+ = 5280 * Vbus(AD) >> 16
#define     TEST_PIN_HIGH           {GPIOD->BSRR |= 0x00000008;}
#define     TEST_PIN_LOW            {GPIOD->BSRR |= 0x00080000;}
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
