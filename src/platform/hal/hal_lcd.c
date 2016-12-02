#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_lcd.h"
#include "font.h"     
#include "debug.h"


LTDC_HandleTypeDef  LTDC_Handler;	    //LTDC句柄
DMA2D_HandleTypeDef DMA2D_Handler; 	    //DMA2D句柄


#pragma location = LCD_FRAME_BUF_ADDR
uint16_t ltdc_lcd_framebuf[1280][800];

//uint16_t ltdc_lcd_framebuf[1280][800] __attribute__((at(LCD_FRAME_BUF_ADDR)));	//定义最大屏分辨率时,LCD所需的帧缓存数组大小

uint32_t *ltdc_framebuf[2];					//LTDC LCD帧缓存数组指针,必须指向对应大小的内存区域
ltdc_dev_t lcdltdc;						//管理LCD LTDC的重要参数

uint32_t POINT_COLOR = 0xFF000000;		//画笔颜色
uint32_t BACK_COLOR = 0xFFFFFFFF;  	//背景色 
  
//管理LCD重要参数
//默认为竖屏
lcd_dev_t lcddev;


void HAL_LTDC_MspInit(LTDC_HandleTypeDef* hltdc)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_LTDC_CLK_ENABLE();                //使能LTDC时钟
    __HAL_RCC_DMA2D_CLK_ENABLE();               //使能DMA2D时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();               //使能GPIOB时钟
    __HAL_RCC_GPIOF_CLK_ENABLE();               //使能GPIOF时钟
    __HAL_RCC_GPIOG_CLK_ENABLE();               //使能GPIOG时钟
    __HAL_RCC_GPIOH_CLK_ENABLE();               //使能GPIOH时钟
    __HAL_RCC_GPIOI_CLK_ENABLE();               //使能GPIOI时钟
    
    //初始化PB5，背光引脚
    GPIO_Initure.Pin = GPIO_PIN_5;                //PB5推挽输出，控制背光
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;      //推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;              //上拉        
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;         //高速
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);
    
    //初始化PF10
    GPIO_Initure.Pin = GPIO_PIN_10; 
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;          //复用
    GPIO_Initure.Pull = GPIO_NOPULL;              
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;         //高速
    GPIO_Initure.Alternate = GPIO_AF14_LTDC;      //复用为LTDC
    HAL_GPIO_Init(GPIOF,&GPIO_Initure);
    
    //初始化PG6,7,11
    GPIO_Initure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOG, &GPIO_Initure);
    
    //初始化PH9,10,11,12,13,14,15
    GPIO_Initure.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);
    
    //初始化PI0,1,2,4,5,6,7,9,10
    GPIO_Initure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOI, &GPIO_Initure); 
}





//打开LCD开关
//lcd_switch:1 打开,0，关闭
void LTDC_Switch(uint8_t sw)
{
	if (sw == 1)
		__HAL_LTDC_ENABLE(&LTDC_Handler);
	else if (sw == 0)
		__HAL_LTDC_DISABLE(&LTDC_Handler);
}

//开关指定层
//layerx:层号,0,第一层; 1,第二层
//sw:1 打开;0关闭
void LTDC_Layer_Switch(uint8_t layerx, uint8_t sw)
{
	if (sw == 1)
		__HAL_LTDC_LAYER_ENABLE(&LTDC_Handler, layerx);
	else if (sw == 0)
		__HAL_LTDC_LAYER_DISABLE(&LTDC_Handler, layerx);
	__HAL_LTDC_RELOAD_CONFIG(&LTDC_Handler);
}

//选择层
//layerx:层号;0,第一层;1,第二层;
void LTDC_Select_Layer(uint8_t layerx)
{
	lcdltdc.activelayer = layerx;
}

//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LTDC_Display_Dir(uint8_t dir)
{
    lcdltdc.dir = dir; 	//显示方向
	if (dir == 0)			//竖屏
	{
		lcdltdc.width = lcdltdc.pheight;
		lcdltdc.height = lcdltdc.pwidth;	
	}
	else if (dir == 1)	//横屏
	{
		lcdltdc.width = lcdltdc.pwidth;
		lcdltdc.height = lcdltdc.pheight;
	}
}

//画点函数
//x,y:坐标
//color:颜色值
void LTDC_Draw_Point(uint16_t x, uint16_t y, uint32_t color)
{ 
	if(lcdltdc.dir)	//横屏
	{
        *(uint16_t *)((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x)) = color;
	}else 			//竖屏
	{
        *(uint16_t *)((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)) = color; 
	}
}

//读点函数
//返回值:颜色值
uint32_t LTDC_Read_Point(uint16_t x, uint16_t y)
{ 
	if (lcdltdc.dir)	//横屏
	{
		return *(uint16_t *)((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x));
	}
	else 			//竖屏
	{
		return *(uint16_t *)((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)); 
	}

}

//LTDC填充矩形,DMA2D填充
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//注意:sx,ex,不能大于lcddev.width-1;sy,ey,不能大于lcddev.height-1!!!
//color:要填充的颜色
void LTDC_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color)
{ 
	uint32_t psx, psy, pex, pey;	//以LCD面板为基准的坐标系,不随横竖屏变化而变化
	uint32_t timeout = 0; 
	uint16_t offline;
	uint32_t addr; 
	
	//坐标系转换
	if (lcdltdc.dir)	//横屏
	{
		psx = sx;
		psy = sy;
		pex = ex;
		pey = ey;
	}
	else			//竖屏
	{
		psx = sy;
		psy = lcdltdc.pheight - ex - 1;
		pex = ey;
		pey = lcdltdc.pheight - sx - 1;
	} 
	
	offline = lcdltdc.pwidth - (pex - psx + 1);
	addr = ((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * psy + psx));
	
	RCC->AHB1ENR |= 1 << 23;			//使能DM2D时钟
	DMA2D->CR = 3 << 16;				//寄存器到存储器模式
	DMA2D->OPFCCR = LCD_PIXFORMAT;	//设置颜色格式
	DMA2D->OOR = offline;				//设置行偏移 
	DMA2D->CR &= ~(1 << 0);				//先停止DMA2D
	DMA2D->OMAR = addr;				//输出存储器地址
	DMA2D->NLR = (pey - psy + 1) | ((pex - psx + 1) << 16);	//设定行数寄存器
	DMA2D->OCOLR = color;				//设定输出颜色寄存器 
	DMA2D->CR |= 1 << 0;				//启动DMA2D

	while ((DMA2D->ISR & (1 << 1)) == 0)	//等待传输完成
	{
		timeout++;
		if (timeout > 0X1FFFFF)
			break;	//超时退出
	} 
	
	DMA2D->IFCR |= 1 << 1;				//清除传输完成标志 	
}


//在指定区域内填充指定颜色块,DMA2D填充	
//此函数仅支持u16,RGB565格式的颜色数组填充.
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)  
//注意:sx,ex,不能大于lcddev.width-1;sy,ey,不能大于lcddev.height-1!!!
//color:要填充的颜色数组首地址
void LTDC_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
	uint32_t psx, psy, pex, pey;	//以LCD面板为基准的坐标系,不随横竖屏变化而变化
	uint32_t timeout = 0; 
	uint16_t offline;
	uint32_t addr; 
	
	//坐标系转换
	if (lcdltdc.dir)	//横屏
	{
		psx = sx;
		psy = sy;
		pex = ex;
		pey = ey;
	}
	else			//竖屏
	{
		psx = sy;
		psy = lcdltdc.pheight - ex - 1;
		pex = ey;
		pey = lcdltdc.pheight - sx - 1;
	}
	
	offline = lcdltdc.pwidth - (pex - psx + 1);
	addr = ((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * psy + psx));
	
	RCC->AHB1ENR |= 1 << 23;			//使能DM2D时钟
	DMA2D->CR = 0 << 16;				//存储器到存储器模式
	DMA2D->FGPFCCR = LCD_PIXFORMAT;	//设置颜色格式
	DMA2D->FGOR = 0;					//前景层行偏移为0
	DMA2D->OOR = offline;				//设置行偏移 
	DMA2D->CR &= ~(1 << 0);				//先停止DMA2D
	DMA2D->FGMAR = (uint32_t)color;		//源地址
	DMA2D->OMAR = addr;				//输出存储器地址
	DMA2D->NLR = (pey - psy + 1) | ((pex - psx + 1) << 16);	//设定行数寄存器 
	DMA2D->CR |= 1 << 0;				//启动DMA2D
	
	while((DMA2D->ISR & (1 << 1)) == 0)	//等待传输完成
	{
		timeout++;
		if (timeout > 0X1FFFFF)
			break;	//超时退出
	}
	
	DMA2D->IFCR |= 1 << 1;				//清除传输完成标志  	
} 

//LCD清屏
//color:颜色值
void LTDC_Clear(uint32_t color)
{
	LTDC_Fill(0, 0, lcdltdc.width - 1, lcdltdc.height - 1, color);
}

//LTDC时钟(Fdclk)设置函数
//Fvco=Fin*pllsain; 
//Fdclk=Fvco/pllsair/2*2^pllsaidivr=Fin*pllsain/pllsair/2*2^pllsaidivr;

//Fvco:VCO频率
//Fin:输入时钟频率一般为1Mhz(来自系统时钟PLLM分频后的时钟,见时钟树图)
//pllsain:SAI时钟倍频系数N,取值范围:50~432.  
//pllsair:SAI时钟的分频系数R,取值范围:2~7
//pllsaidivr:LCD时钟分频系数,取值范围:0~3,对应分频2~16 
//假设:外部晶振为25M,pllm=25的时候,Fin=1Mhz.
//例如:要得到20M的LTDC时钟,则可以设置:pllsain=400,pllsair=5,pllsaidivr=1
//Fdclk=1*396/3/2*2^1=396/12=33Mhz
//返回值:0,成功;1,失败。
uint8_t LTDC_Clk_Set(uint32_t pllsain, uint32_t pllsair, uint32_t pllsaidivr)
{
	RCC_PeriphCLKInitTypeDef PeriphClkIniture;
	
	//LTDC输出像素时钟，需要根据自己所使用的LCD数据手册来配置！
    PeriphClkIniture.PeriphClockSelection = RCC_PERIPHCLK_LTDC;	//LTDC时钟 	
	PeriphClkIniture.PLLSAI.PLLSAIN = pllsain;    
	PeriphClkIniture.PLLSAI.PLLSAIR = pllsair;  
	PeriphClkIniture.PLLSAIDivR = pllsaidivr;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkIniture) == HAL_OK) //配置像素时钟，这里配置为时钟为18.75MHZ
        return 0;   //成功
    else 
		return 1;  //失败    
}

//LTDC,层颜窗口设置,窗口以LCD面板坐标系为基准
//注意:此函数必须在LTDC_Layer_Parameter_Config之后再设置.
//layerx:层值,0/1.
//sx,sy:起始坐标
//width,height:宽度和高度
void LTDC_Layer_Window_Config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    HAL_LTDC_SetWindowPosition(&LTDC_Handler, sx, sy, layerx);  //设置窗口的位置
    HAL_LTDC_SetWindowSize(&LTDC_Handler, width, height, layerx);//设置窗口大小    
}

//LTDC,基本参数设置.
//注意:此函数,必须在LTDC_Layer_Window_Config之前设置.
//layerx:层值,0/1.
//bufaddr:层颜色帧缓存起始地址
//pixformat:颜色格式.0,ARGB8888;1,RGB888;2,RGB565;3,ARGB1555;4,ARGB4444;5,L8;6;AL44;7;AL88
//alpha:层颜色Alpha值,0,全透明;255,不透明
//alpha0:默认颜色Alpha值,0,全透明;255,不透明
//bfac1:混合系数1,4(100),恒定的Alpha;6(101),像素Alpha*恒定Alpha
//bfac2:混合系数2,5(101),恒定的Alpha;7(111),像素Alpha*恒定Alpha
//bkcolor:层默认颜色,32位,低24位有效,RGB888格式
//返回值:无
void LTDC_Layer_Parameter_Config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, 
											uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor)
{
	LTDC_LayerCfgTypeDef pLayerCfg;
	
	pLayerCfg.WindowX0 = 0;                       //窗口起始X坐标
	pLayerCfg.WindowY0 = 0;                       //窗口起始Y坐标
	pLayerCfg.WindowX1 = lcdltdc.pwidth;          //窗口终止X坐标
	pLayerCfg.WindowY1 = lcdltdc.pheight;         //窗口终止Y坐标
	pLayerCfg.PixelFormat = pixformat;		    //像素格式
	pLayerCfg.Alpha = alpha;				        //Alpha值设置，0~255,255为完全不透明
	pLayerCfg.Alpha0 = alpha0;			        //默认Alpha值
	pLayerCfg.BlendingFactor1 = (uint32_t)bfac1 << 8;    //设置层混合系数
	pLayerCfg.BlendingFactor2 = (uint32_t)bfac2 << 8;	//设置层混合系数
	pLayerCfg.FBStartAdress = bufaddr;	        //设置层颜色帧缓存起始地址
	pLayerCfg.ImageWidth = lcdltdc.pwidth;        //设置颜色帧缓冲区的宽度    
	pLayerCfg.ImageHeight = lcdltdc.pheight;      //设置颜色帧缓冲区的高度
	pLayerCfg.Backcolor.Red = (uint8_t)(bkcolor & 0X00FF0000) >> 16;   //背景颜色红色部分
	pLayerCfg.Backcolor.Green = (uint8_t)(bkcolor & 0X0000FF00) >> 8;  //背景颜色绿色部分
	pLayerCfg.Backcolor.Blue = (uint8_t)bkcolor & 0X000000FF;        //背景颜色蓝色部分
	HAL_LTDC_ConfigLayer(&LTDC_Handler, &pLayerCfg, layerx);   //设置所选中的层
}  

//读取面板参数
//PG6=R7(M0);PI2=G7(M1);PI7=B7(M2);
//M2:M1:M0
//0 :0 :0	//4.3寸480*272 RGB屏,ID=0X4342
//0 :0 :1	//7寸800*480 RGB屏,ID=0X7084
//0 :1 :0	//7寸1024*600 RGB屏,ID=0X7016
//0 :1 :1	//7寸1280*800 RGB屏,ID=0X7018
///1 :0 :0	//8寸1024*768 RGB屏,ID=0X8017 
//返回值:LCD ID:0,非法;其他值,ID;
uint16_t LTDC_PanelID_Read(void)
{
	uint8_t idx = 0;
    GPIO_InitTypeDef GPIO_Initure;
	
    __HAL_RCC_GPIOG_CLK_ENABLE();       //使能GPIOG时钟
	__HAL_RCC_GPIOI_CLK_ENABLE();       //使能GPIOI时钟
    
    GPIO_Initure.Pin = GPIO_PIN_6;        //PG6
    GPIO_Initure.Mode = GPIO_MODE_INPUT;  //输入
    GPIO_Initure.Pull = GPIO_PULLUP;      //上拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH; //高速
    HAL_GPIO_Init(GPIOG, &GPIO_Initure); //初始化
    
    GPIO_Initure.Pin = GPIO_PIN_2 | GPIO_PIN_7; //PI2,7
    HAL_GPIO_Init(GPIOI, &GPIO_Initure);     //初始化
    
    idx = (uint8_t)HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_6); //读取M0
    idx |= (uint8_t)HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_2) << 1;//读取M1
    idx |= (uint8_t)HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_7) << 2;//读取M2
    
	if (idx == 0)
		return 0X4342;	//4.3寸屏,480*272分辨率
	else if (idx == 1)
		return 0X7084;	//7寸屏,800*480分辨率
	else if (idx == 2)
		return 0X7016;	//7寸屏,1024*600分辨率
	else if (idx == 3)
		return 0X7018;	//7寸屏,1280*800分辨率
	else if (idx == 4)
		return 0X8017; 	//8寸屏,1024*768分辨率
	else 
		return 0;
}


void LTDC_Init(void)
{   
	uint16_t lcdid = 0;
	
	lcdid = LTDC_PanelID_Read();			//读取LCD面板ID	
	if (lcdid == 0X7016)		
	{
		lcdltdc.pwidth=1024;			//面板宽度,单位:像素
		lcdltdc.pheight=600;			//面板高度,单位:像素
        lcdltdc.hsw=20;				    //水平同步宽度
		lcdltdc.vsw=3;				    //垂直同步宽度
		lcdltdc.hbp=140;			    //水平后廊
		lcdltdc.vbp=20;				    //垂直后廊
		lcdltdc.hfp=160;			    //水平前廊
		lcdltdc.vfp=12;				    //垂直前廊
		LTDC_Clk_Set(360,2,RCC_PLLSAIDIVR_4);//设置像素时钟  45Mhz 
		//其他参数待定.
	}
	else
	{
		DBG_ERROR("LTDC_PanelID_Read error\n");
		return;
	}
	
	lcddev.width = lcdltdc.pwidth;
	lcddev.height = lcdltdc.pheight;
    

    lcdltdc.pixsize = 2;				//每个像素占2个字节
	ltdc_framebuf[0] = (uint32_t *)&ltdc_lcd_framebuf;
    
    //LTDC配置
    LTDC_Handler.Instance = LTDC;
    LTDC_Handler.Init.HSPolarity = LTDC_HSPOLARITY_AL;         //水平同步极性
    LTDC_Handler.Init.VSPolarity = LTDC_VSPOLARITY_AL;         //垂直同步极性
    LTDC_Handler.Init.DEPolarity = LTDC_DEPOLARITY_AL;         //数据使能极性
    LTDC_Handler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;        //像素时钟极性
    LTDC_Handler.Init.HorizontalSync = lcdltdc.hsw - 1;          //水平同步宽度
    LTDC_Handler.Init.VerticalSync = lcdltdc.vsw - 1;            //垂直同步宽度
    LTDC_Handler.Init.AccumulatedHBP = lcdltdc.hsw + lcdltdc.hbp - 1; //水平同步后沿宽度
    LTDC_Handler.Init.AccumulatedVBP = lcdltdc.vsw + lcdltdc.vbp - 1; //垂直同步后沿高度
    LTDC_Handler.Init.AccumulatedActiveW = lcdltdc.hsw+lcdltdc.hbp + lcdltdc.pwidth - 1;//有效宽度
    LTDC_Handler.Init.AccumulatedActiveH = lcdltdc.vsw+lcdltdc.vbp + lcdltdc.pheight - 1;//有效高度
    LTDC_Handler.Init.TotalWidth = lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth + lcdltdc.hfp - 1;   //总宽度
    LTDC_Handler.Init.TotalHeigh = lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight + lcdltdc.vfp - 1;  //总高度
    LTDC_Handler.Init.Backcolor.Red = 0;           //屏幕背景层红色部分
    LTDC_Handler.Init.Backcolor.Green = 0;         //屏幕背景层绿色部分
    LTDC_Handler.Init.Backcolor.Blue = 0;          //屏幕背景色蓝色部分
    HAL_LTDC_Init(&LTDC_Handler);
 	
	//层配置
	LTDC_Layer_Parameter_Config(0, (uint32_t)ltdc_framebuf[0], LCD_PIXFORMAT, 255, 0, 6, 7, 0X000000);	//层参数配置
	LTDC_Layer_Window_Config(0, 0, 0, lcdltdc.pwidth, lcdltdc.pheight);	//层窗口配置,以LCD面板坐标系为基准,不要随便修改!	
	 	
 	LTDC_Display_Dir(0);			//默认竖屏
	LTDC_Select_Layer(0); 			//选择第1层
    LCD_LED(1);         		    //点亮背光
    LTDC_Clear(0XFFFFFFFF);			//清屏
}


//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
uint32_t LCD_ReadPoint(uint16_t x, uint16_t y)
{
 	uint16_t r = 0, g = 0, b = 0;
	
	if (x >= lcddev.width || y >= lcddev.height)
		return 0;	//超过了范围,直接返回

	if (lcdltdc.pwidth != 0)							//如果是RGB屏
	{
		return LTDC_Read_Point(x, y);
	}
}	


//LCD开启显示
void LCD_DisplayOn(void)
{					   
	if (lcdltdc.pwidth != 0)
		LTDC_Switch(1);
}	 


//LCD关闭显示
void LCD_DisplayOff(void)
{	   
	if (lcdltdc.pwidth != 0)
		LTDC_Switch(0);
}   


//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(uint16_t x, uint16_t y)
{
	if (lcdltdc.pwidth != 0)//如果是RGB屏
	{
		LTDC_Draw_Point(x, y, POINT_COLOR);
	}
}


//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint32_t color)
{	   
	if (lcdltdc.pwidth != 0)//如果是RGB屏
	{
		LTDC_Draw_Point(x, y, color);
		return;
	}
}	 


//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(uint8_t dir)
{
    lcddev.dir = dir;         //横屏/竖屏
    
	if (lcdltdc.pwidth != 0)   //如果是RGB屏
	{
		LTDC_Display_Dir(dir);
		
		lcddev.width = lcdltdc.width;
		lcddev.height = lcdltdc.height;
		
		return;
	}
}	 

        
//初始化lcd
//该初始化函数可以初始化各种型号的LCD(详见本.c文件最前面的描述)
void LCD_Init(void)
{ 	  
    lcddev.id = LTDC_PanelID_Read();	//检查是否有RGB屏接入
	if (lcddev.id != 0)
	{
		LTDC_Init();			    //ID非零,说明有RGB屏接入.
	}

	LCD_Display_Dir(0);		//默认为竖屏
	LCD_LED(1);				//点亮背光
	LCD_Clear(WHITE);
}  


//清屏函数
//color:要清屏的填充色
void LCD_Clear(uint32_t color)
{
	uint32_t index = 0;      
	uint32_t totalpoint = lcddev.width;
	
	if (lcdltdc.pwidth != 0)	//如果是RGB屏
	{
		LTDC_Clear(color);
	}
}  


//在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{          
	uint16_t i, j;
	uint16_t xlen = 0;
	
	if (lcdltdc.pwidth != 0)	//如果是RGB屏
	{
		LTDC_Fill(sx, sy, ex, ey, color);
	}
}  


//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{  
	uint16_t height, width;
	uint16_t i, j;
	
	if (lcdltdc.pwidth != 0)	//如果是RGB屏
	{
		LTDC_Color_Fill(sx, sy, ex, ey, color);
	}
}  


//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	
	int32_t xerr = 0, yerr = 0, delta_x, delta_y, distance; 
	int32_t incx, incy, uRow, uCol; 
	
	delta_x = x2 - x1; //计算坐标增量 
	delta_y = y2 - y1; 
	uRow = x1; 
	uCol = y1;
	
	if (delta_x > 0)
		incx = 1; //设置单步方向 
	else if (delta_x == 0)
		incx = 0;//垂直线 
	else 
	{
		incx = -1;
		delta_x = -delta_x;
	}
	
	if (delta_y > 0)
		incy = 1; 
	else if (delta_y == 0)
		incy = 0;//水平线 
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	
	if (delta_x > delta_y)
		distance = delta_x; //选取基本增量坐标轴 
	else 
		distance = delta_y;
	
	for (t = 0; t <= distance + 1; t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow, uCol);//画点 
		xerr += delta_x ; 
		yerr += delta_y ; 
		if (xerr > distance) 
		{ 
			xerr -= distance; 
			uRow += incx; 
		} 
		if (yerr > distance) 
		{ 
			yerr -= distance; 
			uCol += incy; 
		} 
	}  
}    


//画矩形	  
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1, y1, x2, y1);
	LCD_DrawLine(x1, y1, x1, y2);
	LCD_DrawLine(x1, y2, x2, y2);
	LCD_DrawLine(x2, y1, x2, y2);
}


//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r)
{
	int32_t a,b;
	int32_t di;
	
	a = 0;
	b = r;	  
	di = 3 - (r << 1);             //判断下个点位置的标志

	while (a <= b)
	{
		LCD_DrawPoint(x0 + a, y0 - b);             //5
 		LCD_DrawPoint(x0 + b, y0 - a);             //0           
		LCD_DrawPoint(x0 + b, y0 + a);             //4               
		LCD_DrawPoint(x0 + a, y0 + b);             //6 
		LCD_DrawPoint(x0 - a, y0 + b);             //1       
 		LCD_DrawPoint(x0 - b, y0 + a);             
		LCD_DrawPoint(x0 - a, y0 - b);             //2             
  		LCD_DrawPoint(x0 - b, y0 - a);             //7     
  		
		a++;
		
		//使用Bresenham算法画圆     
		if (di < 0)
			di += 4 * a + 6;	  
		else
		{
			di += 10 + 4 * (a - b);   
			b--;
		} 						    
	}
} 		


//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24/32
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode)
{  							  
    uint8_t temp, t1, t;
	uint16_t y0 = y;
	uint8_t csize = (size / 8 + ((size % 8)?1:0)) * (size / 2);		//得到字体一个字符对应点阵集所占的字节数	
	
 	num = num - ' ';//得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）
 	
	for (t = 0; t < csize; t++)
	{   
		if (size == 12)
			temp = asc2_1206[num][t]; 	 	//调用1206字体
		else if (size == 16)
			temp = asc2_1608[num][t];	//调用1608字体
		else if (size == 24)
			temp = asc2_2412[num][t];	//调用2412字体
		else if (size == 32)
			temp = asc2_3216[num][t];	//调用3216字体
		else 
			return;								//没有的字库

		for (t1 = 0; t1 < 8; t1++)
		{			    
			if (temp & 0x80)
				LCD_Fast_DrawPoint(x, y, POINT_COLOR);
			else if (mode == 0)
				LCD_Fast_DrawPoint(x, y, BACK_COLOR);

			temp <<= 1;
			y++;
			
			if (y >= lcddev.height)
				return;		//超区域了
			if ((y - y0) == size)
			{
				y = y0;
				x++;
				if (x >= lcddev.width)
					return;	//超区域了
				break;
			}
		}  	 
	}  	    	   	 	  
}   


//m^n函数
//返回值:m^n次方.
uint32_t LCD_Pow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	
	while (n--)
		result *= m;
	
	return result;
}	


//显示数字,高位为0,则不显示
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//color:颜色 
//num:数值(0~4294967295);	 
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size)
{         	
	uint8_t t, temp;
	uint8_t enshow = 0;
	
	for (t = 0; t < len; t++)
	{
		temp = (num / LCD_Pow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				LCD_ShowChar(x + (size / 2) * t, y, ' ', size, 0);
				continue;
			}
			else
			{
				enshow = 1; 
			} 
		}
		
	 	LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, 0); 
	}
} 


//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);	 
//len:长度(即要显示的位数)
//size:字体大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.
void LCD_ShowxNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode)
{  
	uint8_t t,temp;
	uint8_t enshow = 0;
	
	for (t = 0; t < len; t++)
	{
		temp = (num / LCD_Pow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				if (mode & 0X80)
					LCD_ShowChar(x + (size / 2) * t, y, '0', size, mode & 0X01);  
				else 
					LCD_ShowChar(x + (size / 2) * t, y, ' ', size, mode & 0X01);  

				continue;
			}
			else 
			{
				enshow = 1; 
			}
		}
		
	 	LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, mode & 0X01); 
	}
} 


//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址		  
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p)
{         
	uint8_t x0 = x;
	
	width += x;
	height += y;
	
    while ((*p <= '~') && (*p >= ' '))//判断是不是非法字符!
    {       
        if (x >= width)
		{
			x = x0;
			y += size;
		}
		
        if (y >= height)
			break;//退出
			
        LCD_ShowChar(x, y, *p, size, 0);

		x += size / 2;
        p++;
    }  
}


void hal_lcd_init(void)
{
	LCD_Init();
}

