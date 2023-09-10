#include "cw32f030_flash.h"
#include "cw32f030.h"

#include "flash.h"
#include "elog.h"

#define TAG "flash"

uint8_t Flash_write(uint32_t Add, uint8_t *dataIn, uint32_t dataSize)
{
    uint8_t retNum;

    /* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
    if (Add + dataSize > FLASH_BASE_ADDR + FLASH_SIZE)
    {
        elog_w(TAG, "data longger flash size");
        return 1;
    }

    /* 长度为0时不继续操作  */
    if (dataSize == 0 || dataIn == NULL)
    {
        elog_w(TAG, "no data to write");
        return 1;
    }

    __disable_irq(); /* 关中断 */

    FLASH_UnlockAllPages(); // 解锁

    retNum = FLASH_WriteBytes(Add, dataIn, dataSize);

    if (retNum)
    {
        elog_w(TAG, "write flash waring!");
    }
    /* Flash 加锁，禁止写Flash控制寄存器 */
    FLASH_LockAllPages();

    __enable_irq(); /* 开中断 */

    return 0;
}

uint8_t Flash_erase(uint32_t sectorNum)
{
    uint8_t retNum;
    if (sectorNum > FLASH_Sector_127)
    {
        elog_w(TAG, "sectorNum error");
        return 1;
    }

    __disable_irq(); /* 关中断 */

    FLASH_UnlockPage(sectorNum); // 解锁

    retNum = FLASH_ErasePage(sectorNum);
    if (retNum)
    {
        elog_e(TAG, "Erase flash error!");
    }
    /* Flash 加锁，禁止写Flash控制寄存器 */
    FLASH_LockAllPages();

    __enable_irq(); /* 开中断 */
    return 0;
}

uint8_t Flash_read(uint32_t Addr, uint8_t *readBuff, uint32_t dataSize)
{
    uint32_t i;

    /* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
    if (Addr + dataSize > FLASH_BASE_ADDR + FLASH_SIZE)
    {
        elog_w(TAG, "read data longger flash size");
        return 1;
    }

    /* 长度为0时不继续操作,否则起始地址为奇地址会出错 */
    if (dataSize == 0 || readBuff == NULL)
    {
        elog_w(TAG, "no data to read");
        return 1;
    }

    for (i = 0; i < dataSize; i++)
    {
        *readBuff++ = *(uint8_t *)Addr++;
    }
    return 0;
}
