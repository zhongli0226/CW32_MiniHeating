/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2023-08-12 21:44:35
 * @LastEditors: tangwc
 * @LastEditTime: 2023-09-24 15:42:29
 * @FilePath: \2.Firmware\User\MAX6675\max6675.h
 * 
 *  Copyright (c) 2023 by tangwc, All Rights Reserved. 
 */
#ifndef __MAX6675_H
#define __MAX6675_H

/**
 * @description: MAX6675 gpio 初始化
 * @return {*}
 */
void MAX6675_Init(void);

/**
 * @description: 读取max6675的温度
 * @param {uint16_t} *temp 整数部分温度
 * @return {*} 连接状态
 */
uint16_t MAX6675_Read(uint16_t *temp);


#endif
