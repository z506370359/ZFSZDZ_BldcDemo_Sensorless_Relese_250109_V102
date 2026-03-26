/***********************************************************************
* @file           PI.h
* @author         ZhaoYun(ZFSZDZ)
* @date           2024.12.24
* @Version        V1.0.0
* @description    PI控制器计算模块
***********************************************************************/
#include "../Include/Define.h"

void PIInit(tPI_Type *PIStrcut,signed short Kp,signed short Ki,signed short Kc,signed short Max,signed short Min);
void PIControl(tPI_Type *PIStrcut);

/*******************Copyright (c) ZFSZDZ ******END OF FILE************/
