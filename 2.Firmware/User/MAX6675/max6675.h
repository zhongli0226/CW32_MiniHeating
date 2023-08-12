#ifndef __MAX6675_H
#define __MAX6675_H

/**
 * @description: MAX6675 gpio 初始化
 * @return {*}
 */
void MAX6675_Init(void);

/**
 * @description: 读取max6675的温度
 * @return {*} 整数部分温度
 */
uint16_t MAX6675_Read(void);


#endif
