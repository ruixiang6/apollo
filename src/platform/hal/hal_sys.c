#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_sys.h"
#include "platform.h"


//使能CPU的L1-Cache
void Cache_Enable(void)
{
    SCB_EnableICache();//使能I-Cache
    SCB_EnableDCache();//使能D-Cache    
	SCB->CACR |= 1 << 2;   //强制D-Cache透写,如不开启,实际使用中可能遇到各种问题
}


//时钟设置函数
//Fvco=Fs*(plln/pllm);
//Fsys=Fvco/pllp=Fs*(plln/(pllm*pllp));
//Fusb=Fvco/pllq=Fs*(plln/(pllm*pllq));

//Fvco:VCO频率
//Fsys:系统时钟频率
//Fusb:USB,SDIO,RNG等的时钟频率
//Fs:PLL输入时钟频率,可以是HSI,HSE等. 
//plln:主PLL倍频系数(PLL倍频),取值范围:64~432.
//pllm:主PLL和音频PLL分频系数(PLL之前的分频),取值范围:2~63.
//pllp:系统时钟的主PLL分频系数(PLL之后的分频),取值范围:2,4,6,8.(仅限这4个值!)
//pllq:USB/SDIO/随机数产生器等的主PLL分频系数(PLL之后的分频),取值范围:2~15.

//外部晶振为25M的时候,推荐值:plln=432,pllm=25,pllp=2,pllq=9.
//得到:Fvco=25*(432/25)=432Mhz
//     Fsys=432/2=216Mhz
//     Fusb=432/9=48Mhz
//返回值:0,成功;1,失败
void Stm32_Clock_Init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    HAL_StatusTypeDef ret = HAL_OK;
    RCC_OscInitTypeDef RCC_OscInitStructure; 
    RCC_ClkInitTypeDef RCC_ClkInitStructure;
	
    __HAL_RCC_PWR_CLK_ENABLE(); //使能PWR时钟
 
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);//设置调压器输出电压级别，以便在器件未以最大频率工作
      
    RCC_OscInitStructure.OscillatorType = RCC_OSCILLATORTYPE_HSE;    //时钟源为HSE
    RCC_OscInitStructure.HSEState = RCC_HSE_ON;                      //打开HSE
    RCC_OscInitStructure.PLL.PLLState = RCC_PLL_ON;				   //打开PLL
    RCC_OscInitStructure.PLL.PLLSource = RCC_PLLSOURCE_HSE;          //PLL时钟源选择HSE
    RCC_OscInitStructure.PLL.PLLM = pllm;	//主PLL和音频PLL分频系数(PLL之前的分频)
    RCC_OscInitStructure.PLL.PLLN = plln; //主PLL倍频系数(PLL倍频)
    RCC_OscInitStructure.PLL.PLLP = pllp; //系统时钟的主PLL分频系数(PLL之后的分频)
    RCC_OscInitStructure.PLL.PLLQ = pllq; //USB/SDIO/随机数产生器等的主PLL分频系数(PLL之后的分频)
    ret = HAL_RCC_OscConfig(&RCC_OscInitStructure);//初始化
    if (ret != HAL_OK) 
		while (1);
    
    ret = HAL_PWREx_EnableOverDrive(); //开启Over-Driver功能
    if (ret != HAL_OK) 
		while (1);
    
    //选中PLL作为系统时钟源并且配置HCLK,PCLK1和PCLK2
    RCC_ClkInitStructure.ClockType=(RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStructure.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;//设置系统时钟时钟源为PLL
    RCC_ClkInitStructure.AHBCLKDivider = RCC_SYSCLK_DIV1;//AHB分频系数为1
    RCC_ClkInitStructure.APB1CLKDivider = RCC_HCLK_DIV4;//APB1分频系数为4
    RCC_ClkInitStructure.APB2CLKDivider = RCC_HCLK_DIV2;//APB2分频系数为2
    
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStructure, FLASH_LATENCY_7);//同时设置FLASH延时周期为7WS，也就是8个CPU周期。
    if (ret != HAL_OK) 
		while (1);
}


#ifdef  USE_FULL_ASSERT
//当编译提示出错的时候此函数用来报告错误的文件和所在行
//file：指向源文件
//line：指向在文件中的行数
void assert_failed(uint8_t* file, uint32_t line)
{ 
	while (1)
	{
		
	}
}
#endif


//判断I_Cache是否打开
//返回值:0 关闭，1 打开
uint8_t Get_ICahceSta(void)
{
    uint8_t sta;
	
    sta = ((SCB->CCR) >> 17) & 0X01;
	
    return sta;
}

//判断I_Dache是否打开
//返回值:0 关闭，1 打开
uint8_t Get_DCahceSta(void)
{
    uint8_t sta;
	
    sta = ((SCB->CCR) >> 16) & 0X01;
	
    return sta;
}


//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
void WFI_SET(void)  
{  
    asm("WFI");         
} 

//关闭所有中断(但是不包括fault和NMI中断)
void INTX_DISABLE(void)  
{         
    asm("CPSID   I");     
    asm("BX      LR");    
} 


//开启所有中断
void INTX_ENABLE(void)  
{  
    asm("CPSIE   I");     
    asm("BX      LR");    
}  


//设置栈顶地址
//addr:栈顶地址
void MSR_MSP(uint32_t addr)   
{  
    asm("MSR MSP, r0"); //set Main Stack value  
    asm("BX r14");    
} 



void NMI_Handler(void)
{
	
}


void HardFault_Handler(void)
{
	//while (1)
	{
		DBG_PRINTF("HardFault_Handler\r\n");
	}
}


void MemManage_Handler(void)
{ 
	DBG_PRINTF("Mem Access Error!!\r\n"); 	//输出错误信息
	delay_ms(1000);	
	DBG_PRINTF("Soft Reseting...\r\n");		//提示软件重启
	delay_ms(1000);	
	NVIC_SystemReset();					//软复位
}


void BusFault_Handler(void)
{
	while (1)
	{
		
	}
}


void UsageFault_Handler(void)
{
	while (1)
	{

	}
}


void SVC_Handler(void)
{
	
}


void DebugMon_Handler(void)
{
	
}

//下面两个函数在跑ucos时，由ucos以OS_CPU_PendSVHandler和OS_CPU_SysTickHandler代替之，并修改启动汇编
/*
void PendSV_Handler(void)
{
	
}


void SysTick_Handler(void)
{
	HAL_IncTick();
}
*/


uint8_t MPU_Set_Protection(uint32_t baseaddr, uint32_t size, uint32_t rnum, uint32_t ap)
{
	MPU_Region_InitTypeDef MPU_Initure;
	
	HAL_MPU_Disable();								        //配置MPU之前先关闭MPU,配置完成以后在使能MPU

	MPU_Initure.Enable = MPU_REGION_ENABLE;			        //使能该保护区域 
	MPU_Initure.Number = rnum;			                    //设置保护区域
	MPU_Initure.BaseAddress = baseaddr;	                    //设置基址
	MPU_Initure.Size = size;				                    //设置保护区域大小
	MPU_Initure.SubRegionDisable = 0X00;                      //禁止子区域
	MPU_Initure.TypeExtField = MPU_TEX_LEVEL0;                //设置类型扩展域为level0
	MPU_Initure.AccessPermission = (uint8_t)ap;		            //设置访问权限,
	MPU_Initure.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;	//允许指令访问(允许读取指令)
	MPU_Initure.IsShareable = MPU_ACCESS_NOT_SHAREABLE;       //禁止共用
	MPU_Initure.IsCacheable = MPU_ACCESS_CACHEABLE;       //允许cache  
	MPU_Initure.IsBufferable = MPU_ACCESS_BUFFERABLE;         //允许缓冲
	HAL_MPU_ConfigRegion(&MPU_Initure);                     //配置MPU
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);			        //开启MPU

	return 0;
}


//设置需要保护的存储块
//必须对部分存储区域进行MPU保护,否则可能导致程序运行异常
//比如MCU屏不显示,摄像头采集数据出错等等问题...
void MPU_Memory_Protection(void)
{
	MPU_Set_Protection(0x60000000,MPU_REGION_SIZE_64MB,MPU_REGION_NUMBER0,MPU_REGION_FULL_ACCESS);	//保护MCU LCD屏所在的FMC区域,,共64M字节
	MPU_Set_Protection(0x20000000,MPU_REGION_SIZE_512KB,MPU_REGION_NUMBER1,MPU_REGION_FULL_ACCESS);		//保护整个内部SRAM,包括SRAM1,SRAM2和DTCM,共512K字节
	MPU_Set_Protection(0XC0000000,MPU_REGION_SIZE_32MB,MPU_REGION_NUMBER2,MPU_REGION_FULL_ACCESS);	//保护SDRAM区域,共32M字节
	MPU_Set_Protection(0X80000000,MPU_REGION_SIZE_256MB,MPU_REGION_NUMBER3,MPU_REGION_FULL_ACCESS);	//保护整个NAND FLASH区域,共256M字节
}



void hal_broad_init(void)
{
	Cache_Enable();
	
	MPU_Memory_Protection();

	HAL_Init();

	Stm32_Clock_Init(432, 25, 2, 9);   //设置时钟,216Mhz 
}
