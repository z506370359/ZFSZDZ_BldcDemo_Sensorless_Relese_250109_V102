/***********************************************************************
* @file           Define.h
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.10.12
* @Version        V1.0.0
* @description    张飞实战电子"扬帆起航"课程无感方波控制
***********************************************************************/

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色
#define GRAYBLUE       	 0X5458 //灰蓝色
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

/*=========================================================================*/
typedef struct
{
    unsigned short ADDATA0;
    unsigned short ADDATA1;
    unsigned short ADDATA2;
    unsigned short ADDATA3;
    unsigned short ADDATA4;
}tDMA_Type;
/*=========================================================================*/
typedef struct
{
    unsigned short ValueInstant; //数据瞬时值
    unsigned short ValueAverage; //数据平均值
    unsigned short Num; //求平均值计数
    unsigned int Sum; //求平均值累加和
}tAnx_Type;
/*=========================================================================*/
typedef struct
{
    signed char NtcReal;//温度真实值
    tAnx_Type An0,An3,An4,An5,An9;
    unsigned short SetDuty; //AD模块占空比调节原始值
    unsigned short TerminalVoltage;//ADC模块采样原始端电压
    unsigned short Vbus; //Vbus电压AD采样值
    unsigned short SetSpeed;//通过电位器旋钮设置目标转速
    unsigned short VbusReal; //Vbus电压真实值 放大10倍
}tAdc_Type;
/*=========================================================================*/
typedef struct
{
    unsigned short StartDelayCount; //系统上电延时计数变量
    unsigned short LcdCount; //LCD刷新计数变量
    unsigned short SpeedAdjustCount;//目标转速斜坡变化时间计数变量
    unsigned short UsartCount; //串口发送一帧数据计数变量
    unsigned short DutyCount; //开环模式下占空比斜坡计数变量
}tSystem_Type;
/*=========================================================================*/
typedef struct
{
    unsigned char State; //状态机状态变量
    unsigned char PhaseStep; //相位变量
    unsigned char DemagnetizationTime; //退磁时间
    unsigned char ForceState; //强拖运行状态变量
    unsigned char SensorlessState; //无感运行状态变量
    unsigned char Slope; //悬空相端电压变化趋势 1 上坡 0下坡
    unsigned char ZeroPointCount;//过零点计数器
    unsigned char ZeventTimeNum; //60°时间存储计数变量
    unsigned char SpeedUpdate;//360度计数值获取后，用于速度求取
    unsigned char SpeedEnable;//目标速度更新使能
    unsigned short Duty; //占空比赋值变量
    unsigned short AimDuty; //速度环输出占空比值
    unsigned short PwmCount;// PWM周期计数器
    unsigned short ForcePhaseTimeBase; //强拖换相时间基准值
    unsigned short ForcePhaseTimeCount; //强拖换相时间计数值
    unsigned short TerminalVoltageMax; //端电压最大阈值
    unsigned short TerminalVoltageMin; //端电压最小阈值
    unsigned short TerminalVoltage;//ADC模块采样原始端电压
    unsigned short VbusZcp;//1/2Vbus反电动势过零比较值
    unsigned short ZeventTime; //相邻过零点时间
    unsigned short CeventTime; //换相时间
    unsigned short ZeventTimeArray[6]; //60度时间存储数组
}tBldc_Type;
/*=========================================================================*/
typedef struct
{
    unsigned char RunStopState; //运行停止状态
    unsigned char Direction; //电机旋转方向
    unsigned char SpeedMode; //速度模式设置 旋钮 or 串口
    unsigned char RunMode; //电机运行模式  开环 or 闭环
    unsigned short InstantSpeed; //电机瞬时速度
    unsigned short AimSpeed; //电机目标转速给定
    unsigned short ActSpeed; //电机实际转速
    unsigned short DisPlaySpeed; //显示转速
    unsigned short VarDelta; //斜坡变化量
    unsigned short AimDuty; //速度环输出占空比值
}tMotor_Type;
/*=========================================================================*/
typedef struct
{
    signed short qKp;//比例项系数 Q15
    signed short qKi;//积分项系数
    signed short qKc;//抗积分饱和系数
    signed short qOut;//PI控制器输出值
    signed short qOutMax; //PI控制器输出限幅最大值
    signed short qOutMin; //PI控制器输出限幅最小值
    signed short qInRef;//目标值
    signed short qInFeb;//反馈值
    signed int qSum;//积分累加项 Q30
}tPI_Type;
/*=========================================================================*/
typedef struct
{
    unsigned char HardOverCurrentCount; //硬件过流计数
    unsigned char Code;//故障代码
    unsigned char SoftOverCurrentCount;//软件过流计数
    unsigned char OverVoltageCount; //过压计数
    unsigned char UnderVoltageCount; //欠压计数
    unsigned char OverTemperatureCount; //过温计数
    unsigned short LockRotorCurrentCount;//堵转电流保护计数
    unsigned short LockRotorPhaseCount;//堵转相位保护计数
    unsigned short StartFailCount; //启动失败计数
}tProtect_Type;
/*==================================================================================*/
typedef struct
{
    unsigned short SendReq:1; //数据发送请求标志
    unsigned short RxStart:1; //开始接收数据标志
    unsigned short Unusual:1; //数据无效标志
    unsigned short ReceReq:1; //接收数据处理请求
    unsigned short TxStart:1; //开始发送一帧数据标志
    unsigned short RESERVED:11; //保留位
}STRUCT_USART_FLAG;
/*==================================================================================*/
typedef struct
{
    unsigned char TxNo; //发送数据包计数变量
    
    STRUCT_USART_FLAG Flag; //串口标志
    unsigned short TxData; //发送数据变量
    unsigned short TxSum; //校验和求取
    unsigned short TxNum; //发送数据个数
    unsigned short RxSum; //校验和求取
    unsigned short RxNum; //接收数据个数
    unsigned short RxDataBuf[3]; //接收有效数据BUF
    unsigned short FmVersion; //固件版本号
    unsigned short Duty; //串口通讯 开环占空比设定
    unsigned short SetSpeed; //串口通讯 闭环给定速度设定
}tUsart_Type;
/*==================================================================================*/
typedef struct
{
    float fdata[2]; //要传输的数据
    unsigned char Tail[4]; //帧尾
}tVOFA_Type;
/*==================================================================================*/
typedef struct
{
    unsigned char Step; //初始位置检测步骤
    unsigned char Count; //绕组通电时间设置
    unsigned char RotorSector;//转子所在的扇区
    unsigned short CurrentValue[6];//记录打脉冲对应的电流
    unsigned short SumIab_Iba;
    unsigned short SumIac_Ica;
    unsigned short SumIbc_Icb;
    
}tIPD_Type;
/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
