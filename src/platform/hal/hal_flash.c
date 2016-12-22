#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "platform.h"
#include "hal_flash.h"
 

uint32_t STMFLASH_ReadWord(uint32_t faddr)
{
	return *(__IO uint32_t *)faddr; 
}

//��ȡĳ����ַ���ڵ�flash����
//addr:flash��ַ
//����ֵ:0~11,��addr���ڵ�����
uint16_t STMFLASH_GetFlashSector(uint32_t addr)
{
	if (addr < ADDR_FLASH_SECTOR_1)
		return FLASH_SECTOR_0;
	else if (addr < ADDR_FLASH_SECTOR_2)
		return FLASH_SECTOR_1;
	else if (addr < ADDR_FLASH_SECTOR_3)
		return FLASH_SECTOR_2;
	else if (addr < ADDR_FLASH_SECTOR_4)
		return FLASH_SECTOR_3;
	else if (addr < ADDR_FLASH_SECTOR_5)
		return FLASH_SECTOR_4;
	else if (addr < ADDR_FLASH_SECTOR_6)
		return FLASH_SECTOR_5;
	else if (addr < ADDR_FLASH_SECTOR_7)
		return FLASH_SECTOR_6;
	return FLASH_SECTOR_7;	
}

//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ر�ע��:��ΪSTM32F7������ʵ��̫��,û�취���ر�����������,���Ա�����
//         д��ַ�����0XFF,��ô���Ȳ������������Ҳ�������������.����
//         д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.����д֮ǰȷ��������
//         û����Ҫ����,��������������Ȳ�����,Ȼ����������д. 
//�ú�����OTP����Ҳ��Ч!��������дOTP��!
//OTP�����ַ��Χ:0X1FF0F000~0X1FF0F41F
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
//pBuffer:����ָ��
//NumToWrite:��(32λ)��(����Ҫд���32λ���ݵĸ���.) 
void STMFLASH_Write(uint32_t WriteAddr,uint32_t *pBuffer,uint32_t NumToWrite)	
{ 
    FLASH_EraseInitTypeDef FlashEraseInit;
    HAL_StatusTypeDef FlashStatus=HAL_OK;
    uint32_t SectorError=0;
	uint32_t addrx=0;
	uint32_t endaddr=0;	
    if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//�Ƿ���ַ
    
 	HAL_FLASH_Unlock();             //����	
	addrx=WriteAddr;				//д�����ʼ��ַ
	endaddr=WriteAddr+NumToWrite*4;	//д��Ľ�����ַ
    
    if(addrx<0X1FF00000)
    {
        while(addrx<endaddr)		//ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
			{   
                FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;       //�������ͣ��������� 
                FlashEraseInit.Sector=STMFLASH_GetFlashSector(addrx);   //Ҫ����������
                FlashEraseInit.NbSectors=1;                             //һ��ֻ����һ������
                FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;      //��ѹ��Χ��VCC=2.7~3.6V֮��!!
                if(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK) 
                {
                    break;//����������	
                }
                SCB_CleanInvalidateDCache();                            //�����Ч��D-Cache
			}else addrx+=4;
            FLASH_WaitForLastOperation(FLASH_WAITETIME);                //�ȴ��ϴβ������
        }
    }
    FlashStatus=FLASH_WaitForLastOperation(FLASH_WAITETIME);            //�ȴ��ϴβ������
	if(FlashStatus==HAL_OK)
	{
		while(WriteAddr<endaddr)//д����
		{
            if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,WriteAddr,*pBuffer)!=HAL_OK)//д������
			{ 
				break;	//д���쳣
			}
			WriteAddr+=4;
			pBuffer++;
		} 
	}
	HAL_FLASH_Lock();           //����
} 


void hal_flash_write(uint32_t flash_ptr, uint8_t *buf, uint16_t len)
{		
	if (flash_ptr >= HAL_FLASH_BASE_ADDR && flash_ptr < HAL_FLASH_END_ADDR)
	{
		if (flash_ptr+len >= HAL_FLASH_BASE_ADDR && flash_ptr+len<HAL_FLASH_END_ADDR)
		{
			STMFLASH_Write(flash_ptr, (uint32_t *)buf, (len/4 + 1));		
		}
	}
	
	return;	
}

void hal_flash_read(uint32_t flash_ptr, uint8_t *buf, uint16_t len)
{
	
	if (flash_ptr >= HAL_FLASH_BASE_ADDR && flash_ptr < HAL_FLASH_END_ADDR)
	{
		if (flash_ptr+len >= HAL_FLASH_BASE_ADDR && flash_ptr+len < HAL_FLASH_END_ADDR)
		{
			mem_cpy((void *)buf, (void *)flash_ptr, len);

			return;
		}
	}
	
	return;
}
