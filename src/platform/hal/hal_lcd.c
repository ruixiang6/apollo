#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "hal_lcd.h"
#include "font.h"     
#include "debug.h"


LTDC_HandleTypeDef  LTDC_Handler;	    //LTDC���
DMA2D_HandleTypeDef DMA2D_Handler; 	    //DMA2D���


#pragma location = LCD_FRAME_BUF_ADDR
uint16_t ltdc_lcd_framebuf[1280][800];

//uint16_t ltdc_lcd_framebuf[1280][800] __attribute__((at(LCD_FRAME_BUF_ADDR)));	//����������ֱ���ʱ,LCD�����֡���������С

uint32_t *ltdc_framebuf[2];					//LTDC LCD֡��������ָ��,����ָ���Ӧ��С���ڴ�����
ltdc_dev_t lcdltdc;						//����LCD LTDC����Ҫ����

uint32_t POINT_COLOR = 0xFF000000;		//������ɫ
uint32_t BACK_COLOR = 0xFFFFFFFF;  	//����ɫ 
  
//����LCD��Ҫ����
//Ĭ��Ϊ����
lcd_dev_t lcddev;


void HAL_LTDC_MspInit(LTDC_HandleTypeDef* hltdc)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_LTDC_CLK_ENABLE();                //ʹ��LTDCʱ��
    __HAL_RCC_DMA2D_CLK_ENABLE();               //ʹ��DMA2Dʱ��
    __HAL_RCC_GPIOB_CLK_ENABLE();               //ʹ��GPIOBʱ��
    __HAL_RCC_GPIOF_CLK_ENABLE();               //ʹ��GPIOFʱ��
    __HAL_RCC_GPIOG_CLK_ENABLE();               //ʹ��GPIOGʱ��
    __HAL_RCC_GPIOH_CLK_ENABLE();               //ʹ��GPIOHʱ��
    __HAL_RCC_GPIOI_CLK_ENABLE();               //ʹ��GPIOIʱ��
    
    //��ʼ��PB5����������
    GPIO_Initure.Pin = GPIO_PIN_5;                //PB5������������Ʊ���
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;      //�������
    GPIO_Initure.Pull = GPIO_PULLUP;              //����        
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;         //����
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);
    
    //��ʼ��PF10
    GPIO_Initure.Pin = GPIO_PIN_10; 
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;          //����
    GPIO_Initure.Pull = GPIO_NOPULL;              
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;         //����
    GPIO_Initure.Alternate = GPIO_AF14_LTDC;      //����ΪLTDC
    HAL_GPIO_Init(GPIOF,&GPIO_Initure);
    
    //��ʼ��PG6,7,11
    GPIO_Initure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOG, &GPIO_Initure);
    
    //��ʼ��PH9,10,11,12,13,14,15
    GPIO_Initure.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);
    
    //��ʼ��PI0,1,2,4,5,6,7,9,10
    GPIO_Initure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOI, &GPIO_Initure); 
}





//��LCD����
//lcd_switch:1 ��,0���ر�
void LTDC_Switch(uint8_t sw)
{
	if (sw == 1)
		__HAL_LTDC_ENABLE(&LTDC_Handler);
	else if (sw == 0)
		__HAL_LTDC_DISABLE(&LTDC_Handler);
}

//����ָ����
//layerx:���,0,��һ��; 1,�ڶ���
//sw:1 ��;0�ر�
void LTDC_Layer_Switch(uint8_t layerx, uint8_t sw)
{
	if (sw == 1)
		__HAL_LTDC_LAYER_ENABLE(&LTDC_Handler, layerx);
	else if (sw == 0)
		__HAL_LTDC_LAYER_DISABLE(&LTDC_Handler, layerx);
	__HAL_LTDC_RELOAD_CONFIG(&LTDC_Handler);
}

//ѡ���
//layerx:���;0,��һ��;1,�ڶ���;
void LTDC_Select_Layer(uint8_t layerx)
{
	lcdltdc.activelayer = layerx;
}

//����LCD��ʾ����
//dir:0,������1,����
void LTDC_Display_Dir(uint8_t dir)
{
    lcdltdc.dir = dir; 	//��ʾ����
	if (dir == 0)			//����
	{
		lcdltdc.width = lcdltdc.pheight;
		lcdltdc.height = lcdltdc.pwidth;	
	}
	else if (dir == 1)	//����
	{
		lcdltdc.width = lcdltdc.pwidth;
		lcdltdc.height = lcdltdc.pheight;
	}
}

//���㺯��
//x,y:����
//color:��ɫֵ
void LTDC_Draw_Point(uint16_t x, uint16_t y, uint32_t color)
{ 
	if(lcdltdc.dir)	//����
	{
        *(uint16_t *)((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x)) = color;
	}else 			//����
	{
        *(uint16_t *)((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)) = color; 
	}
}

//���㺯��
//����ֵ:��ɫֵ
uint32_t LTDC_Read_Point(uint16_t x, uint16_t y)
{ 
	if (lcdltdc.dir)	//����
	{
		return *(uint16_t *)((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * y + x));
	}
	else 			//����
	{
		return *(uint16_t *)((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * (lcdltdc.pheight - x - 1) + y)); 
	}

}

//LTDC������,DMA2D���
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//ע��:sx,ex,���ܴ���lcddev.width-1;sy,ey,���ܴ���lcddev.height-1!!!
//color:Ҫ������ɫ
void LTDC_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color)
{ 
	uint32_t psx, psy, pex, pey;	//��LCD���Ϊ��׼������ϵ,����������仯���仯
	uint32_t timeout = 0; 
	uint16_t offline;
	uint32_t addr; 
	
	//����ϵת��
	if (lcdltdc.dir)	//����
	{
		psx = sx;
		psy = sy;
		pex = ex;
		pey = ey;
	}
	else			//����
	{
		psx = sy;
		psy = lcdltdc.pheight - ex - 1;
		pex = ey;
		pey = lcdltdc.pheight - sx - 1;
	} 
	
	offline = lcdltdc.pwidth - (pex - psx + 1);
	addr = ((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * psy + psx));
	
	RCC->AHB1ENR |= 1 << 23;			//ʹ��DM2Dʱ��
	DMA2D->CR = 3 << 16;				//�Ĵ������洢��ģʽ
	DMA2D->OPFCCR = LCD_PIXFORMAT;	//������ɫ��ʽ
	DMA2D->OOR = offline;				//������ƫ�� 
	DMA2D->CR &= ~(1 << 0);				//��ֹͣDMA2D
	DMA2D->OMAR = addr;				//����洢����ַ
	DMA2D->NLR = (pey - psy + 1) | ((pex - psx + 1) << 16);	//�趨�����Ĵ���
	DMA2D->OCOLR = color;				//�趨�����ɫ�Ĵ��� 
	DMA2D->CR |= 1 << 0;				//����DMA2D

	while ((DMA2D->ISR & (1 << 1)) == 0)	//�ȴ��������
	{
		timeout++;
		if (timeout > 0X1FFFFF)
			break;	//��ʱ�˳�
	} 
	
	DMA2D->IFCR |= 1 << 1;				//���������ɱ�־ 	
}


//��ָ�����������ָ����ɫ��,DMA2D���	
//�˺�����֧��u16,RGB565��ʽ����ɫ�������.
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)  
//ע��:sx,ex,���ܴ���lcddev.width-1;sy,ey,���ܴ���lcddev.height-1!!!
//color:Ҫ������ɫ�����׵�ַ
void LTDC_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
	uint32_t psx, psy, pex, pey;	//��LCD���Ϊ��׼������ϵ,����������仯���仯
	uint32_t timeout = 0; 
	uint16_t offline;
	uint32_t addr; 
	
	//����ϵת��
	if (lcdltdc.dir)	//����
	{
		psx = sx;
		psy = sy;
		pex = ex;
		pey = ey;
	}
	else			//����
	{
		psx = sy;
		psy = lcdltdc.pheight - ex - 1;
		pex = ey;
		pey = lcdltdc.pheight - sx - 1;
	}
	
	offline = lcdltdc.pwidth - (pex - psx + 1);
	addr = ((uint32_t)ltdc_framebuf[lcdltdc.activelayer] + lcdltdc.pixsize * (lcdltdc.pwidth * psy + psx));
	
	RCC->AHB1ENR |= 1 << 23;			//ʹ��DM2Dʱ��
	DMA2D->CR = 0 << 16;				//�洢�����洢��ģʽ
	DMA2D->FGPFCCR = LCD_PIXFORMAT;	//������ɫ��ʽ
	DMA2D->FGOR = 0;					//ǰ������ƫ��Ϊ0
	DMA2D->OOR = offline;				//������ƫ�� 
	DMA2D->CR &= ~(1 << 0);				//��ֹͣDMA2D
	DMA2D->FGMAR = (uint32_t)color;		//Դ��ַ
	DMA2D->OMAR = addr;				//����洢����ַ
	DMA2D->NLR = (pey - psy + 1) | ((pex - psx + 1) << 16);	//�趨�����Ĵ��� 
	DMA2D->CR |= 1 << 0;				//����DMA2D
	
	while((DMA2D->ISR & (1 << 1)) == 0)	//�ȴ��������
	{
		timeout++;
		if (timeout > 0X1FFFFF)
			break;	//��ʱ�˳�
	}
	
	DMA2D->IFCR |= 1 << 1;				//���������ɱ�־  	
} 

//LCD����
//color:��ɫֵ
void LTDC_Clear(uint32_t color)
{
	LTDC_Fill(0, 0, lcdltdc.width - 1, lcdltdc.height - 1, color);
}

//LTDCʱ��(Fdclk)���ú���
//Fvco=Fin*pllsain; 
//Fdclk=Fvco/pllsair/2*2^pllsaidivr=Fin*pllsain/pllsair/2*2^pllsaidivr;

//Fvco:VCOƵ��
//Fin:����ʱ��Ƶ��һ��Ϊ1Mhz(����ϵͳʱ��PLLM��Ƶ���ʱ��,��ʱ����ͼ)
//pllsain:SAIʱ�ӱ�Ƶϵ��N,ȡֵ��Χ:50~432.  
//pllsair:SAIʱ�ӵķ�Ƶϵ��R,ȡֵ��Χ:2~7
//pllsaidivr:LCDʱ�ӷ�Ƶϵ��,ȡֵ��Χ:0~3,��Ӧ��Ƶ2~16 
//����:�ⲿ����Ϊ25M,pllm=25��ʱ��,Fin=1Mhz.
//����:Ҫ�õ�20M��LTDCʱ��,���������:pllsain=400,pllsair=5,pllsaidivr=1
//Fdclk=1*396/3/2*2^1=396/12=33Mhz
//����ֵ:0,�ɹ�;1,ʧ�ܡ�
uint8_t LTDC_Clk_Set(uint32_t pllsain, uint32_t pllsair, uint32_t pllsaidivr)
{
	RCC_PeriphCLKInitTypeDef PeriphClkIniture;
	
	//LTDC�������ʱ�ӣ���Ҫ�����Լ���ʹ�õ�LCD�����ֲ������ã�
    PeriphClkIniture.PeriphClockSelection = RCC_PERIPHCLK_LTDC;	//LTDCʱ�� 	
	PeriphClkIniture.PLLSAI.PLLSAIN = pllsain;    
	PeriphClkIniture.PLLSAI.PLLSAIR = pllsair;  
	PeriphClkIniture.PLLSAIDivR = pllsaidivr;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkIniture) == HAL_OK) //��������ʱ�ӣ���������Ϊʱ��Ϊ18.75MHZ
        return 0;   //�ɹ�
    else 
		return 1;  //ʧ��    
}

//LTDC,���մ�������,������LCD�������ϵΪ��׼
//ע��:�˺���������LTDC_Layer_Parameter_Config֮��������.
//layerx:��ֵ,0/1.
//sx,sy:��ʼ����
//width,height:��Ⱥ͸߶�
void LTDC_Layer_Window_Config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    HAL_LTDC_SetWindowPosition(&LTDC_Handler, sx, sy, layerx);  //���ô��ڵ�λ��
    HAL_LTDC_SetWindowSize(&LTDC_Handler, width, height, layerx);//���ô��ڴ�С    
}

//LTDC,������������.
//ע��:�˺���,������LTDC_Layer_Window_Config֮ǰ����.
//layerx:��ֵ,0/1.
//bufaddr:����ɫ֡������ʼ��ַ
//pixformat:��ɫ��ʽ.0,ARGB8888;1,RGB888;2,RGB565;3,ARGB1555;4,ARGB4444;5,L8;6;AL44;7;AL88
//alpha:����ɫAlphaֵ,0,ȫ͸��;255,��͸��
//alpha0:Ĭ����ɫAlphaֵ,0,ȫ͸��;255,��͸��
//bfac1:���ϵ��1,4(100),�㶨��Alpha;6(101),����Alpha*�㶨Alpha
//bfac2:���ϵ��2,5(101),�㶨��Alpha;7(111),����Alpha*�㶨Alpha
//bkcolor:��Ĭ����ɫ,32λ,��24λ��Ч,RGB888��ʽ
//����ֵ:��
void LTDC_Layer_Parameter_Config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, 
											uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor)
{
	LTDC_LayerCfgTypeDef pLayerCfg;
	
	pLayerCfg.WindowX0 = 0;                       //������ʼX����
	pLayerCfg.WindowY0 = 0;                       //������ʼY����
	pLayerCfg.WindowX1 = lcdltdc.pwidth;          //������ֹX����
	pLayerCfg.WindowY1 = lcdltdc.pheight;         //������ֹY����
	pLayerCfg.PixelFormat = pixformat;		    //���ظ�ʽ
	pLayerCfg.Alpha = alpha;				        //Alphaֵ���ã�0~255,255Ϊ��ȫ��͸��
	pLayerCfg.Alpha0 = alpha0;			        //Ĭ��Alphaֵ
	pLayerCfg.BlendingFactor1 = (uint32_t)bfac1 << 8;    //���ò���ϵ��
	pLayerCfg.BlendingFactor2 = (uint32_t)bfac2 << 8;	//���ò���ϵ��
	pLayerCfg.FBStartAdress = bufaddr;	        //���ò���ɫ֡������ʼ��ַ
	pLayerCfg.ImageWidth = lcdltdc.pwidth;        //������ɫ֡�������Ŀ��    
	pLayerCfg.ImageHeight = lcdltdc.pheight;      //������ɫ֡�������ĸ߶�
	pLayerCfg.Backcolor.Red = (uint8_t)(bkcolor & 0X00FF0000) >> 16;   //������ɫ��ɫ����
	pLayerCfg.Backcolor.Green = (uint8_t)(bkcolor & 0X0000FF00) >> 8;  //������ɫ��ɫ����
	pLayerCfg.Backcolor.Blue = (uint8_t)bkcolor & 0X000000FF;        //������ɫ��ɫ����
	HAL_LTDC_ConfigLayer(&LTDC_Handler, &pLayerCfg, layerx);   //������ѡ�еĲ�
}  

//��ȡ������
//PG6=R7(M0);PI2=G7(M1);PI7=B7(M2);
//M2:M1:M0
//0 :0 :0	//4.3��480*272 RGB��,ID=0X4342
//0 :0 :1	//7��800*480 RGB��,ID=0X7084
//0 :1 :0	//7��1024*600 RGB��,ID=0X7016
//0 :1 :1	//7��1280*800 RGB��,ID=0X7018
///1 :0 :0	//8��1024*768 RGB��,ID=0X8017 
//����ֵ:LCD ID:0,�Ƿ�;����ֵ,ID;
uint16_t LTDC_PanelID_Read(void)
{
	uint8_t idx = 0;
    GPIO_InitTypeDef GPIO_Initure;
	
    __HAL_RCC_GPIOG_CLK_ENABLE();       //ʹ��GPIOGʱ��
	__HAL_RCC_GPIOI_CLK_ENABLE();       //ʹ��GPIOIʱ��
    
    GPIO_Initure.Pin = GPIO_PIN_6;        //PG6
    GPIO_Initure.Mode = GPIO_MODE_INPUT;  //����
    GPIO_Initure.Pull = GPIO_PULLUP;      //����
    GPIO_Initure.Speed = GPIO_SPEED_HIGH; //����
    HAL_GPIO_Init(GPIOG, &GPIO_Initure); //��ʼ��
    
    GPIO_Initure.Pin = GPIO_PIN_2 | GPIO_PIN_7; //PI2,7
    HAL_GPIO_Init(GPIOI, &GPIO_Initure);     //��ʼ��
    
    idx = (uint8_t)HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_6); //��ȡM0
    idx |= (uint8_t)HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_2) << 1;//��ȡM1
    idx |= (uint8_t)HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_7) << 2;//��ȡM2
    
	if (idx == 0)
		return 0X4342;	//4.3����,480*272�ֱ���
	else if (idx == 1)
		return 0X7084;	//7����,800*480�ֱ���
	else if (idx == 2)
		return 0X7016;	//7����,1024*600�ֱ���
	else if (idx == 3)
		return 0X7018;	//7����,1280*800�ֱ���
	else if (idx == 4)
		return 0X8017; 	//8����,1024*768�ֱ���
	else 
		return 0;
}


void LTDC_Init(void)
{   
	uint16_t lcdid = 0;
	
	lcdid = LTDC_PanelID_Read();			//��ȡLCD���ID	
	if (lcdid == 0X7016)		
	{
		lcdltdc.pwidth=1024;			//�����,��λ:����
		lcdltdc.pheight=600;			//���߶�,��λ:����
        lcdltdc.hsw=20;				    //ˮƽͬ�����
		lcdltdc.vsw=3;				    //��ֱͬ�����
		lcdltdc.hbp=140;			    //ˮƽ����
		lcdltdc.vbp=20;				    //��ֱ����
		lcdltdc.hfp=160;			    //ˮƽǰ��
		lcdltdc.vfp=12;				    //��ֱǰ��
		LTDC_Clk_Set(360,2,RCC_PLLSAIDIVR_4);//��������ʱ��  45Mhz 
		//������������.
	}
	else
	{
		DBG_ERROR("LTDC_PanelID_Read error\n");
		return;
	}
	
	lcddev.width = lcdltdc.pwidth;
	lcddev.height = lcdltdc.pheight;
    

    lcdltdc.pixsize = 2;				//ÿ������ռ2���ֽ�
	ltdc_framebuf[0] = (uint32_t *)&ltdc_lcd_framebuf;
    
    //LTDC����
    LTDC_Handler.Instance = LTDC;
    LTDC_Handler.Init.HSPolarity = LTDC_HSPOLARITY_AL;         //ˮƽͬ������
    LTDC_Handler.Init.VSPolarity = LTDC_VSPOLARITY_AL;         //��ֱͬ������
    LTDC_Handler.Init.DEPolarity = LTDC_DEPOLARITY_AL;         //����ʹ�ܼ���
    LTDC_Handler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;        //����ʱ�Ӽ���
    LTDC_Handler.Init.HorizontalSync = lcdltdc.hsw - 1;          //ˮƽͬ�����
    LTDC_Handler.Init.VerticalSync = lcdltdc.vsw - 1;            //��ֱͬ�����
    LTDC_Handler.Init.AccumulatedHBP = lcdltdc.hsw + lcdltdc.hbp - 1; //ˮƽͬ�����ؿ��
    LTDC_Handler.Init.AccumulatedVBP = lcdltdc.vsw + lcdltdc.vbp - 1; //��ֱͬ�����ظ߶�
    LTDC_Handler.Init.AccumulatedActiveW = lcdltdc.hsw+lcdltdc.hbp + lcdltdc.pwidth - 1;//��Ч���
    LTDC_Handler.Init.AccumulatedActiveH = lcdltdc.vsw+lcdltdc.vbp + lcdltdc.pheight - 1;//��Ч�߶�
    LTDC_Handler.Init.TotalWidth = lcdltdc.hsw + lcdltdc.hbp + lcdltdc.pwidth + lcdltdc.hfp - 1;   //�ܿ��
    LTDC_Handler.Init.TotalHeigh = lcdltdc.vsw + lcdltdc.vbp + lcdltdc.pheight + lcdltdc.vfp - 1;  //�ܸ߶�
    LTDC_Handler.Init.Backcolor.Red = 0;           //��Ļ�������ɫ����
    LTDC_Handler.Init.Backcolor.Green = 0;         //��Ļ��������ɫ����
    LTDC_Handler.Init.Backcolor.Blue = 0;          //��Ļ����ɫ��ɫ����
    HAL_LTDC_Init(&LTDC_Handler);
 	
	//������
	LTDC_Layer_Parameter_Config(0, (uint32_t)ltdc_framebuf[0], LCD_PIXFORMAT, 255, 0, 6, 7, 0X000000);	//���������
	LTDC_Layer_Window_Config(0, 0, 0, lcdltdc.pwidth, lcdltdc.pheight);	//�㴰������,��LCD�������ϵΪ��׼,��Ҫ����޸�!	
	 	
 	LTDC_Display_Dir(0);			//Ĭ������
	LTDC_Select_Layer(0); 			//ѡ���1��
    LCD_LED(1);         		    //��������
    LTDC_Clear(0XFFFFFFFF);			//����
}


//��ȡ��ĳ�����ɫֵ	 
//x,y:����
//����ֵ:�˵����ɫ
uint32_t LCD_ReadPoint(uint16_t x, uint16_t y)
{
 	uint16_t r = 0, g = 0, b = 0;
	
	if (x >= lcddev.width || y >= lcddev.height)
		return 0;	//�����˷�Χ,ֱ�ӷ���

	if (lcdltdc.pwidth != 0)							//�����RGB��
	{
		return LTDC_Read_Point(x, y);
	}
}	


//LCD������ʾ
void LCD_DisplayOn(void)
{					   
	if (lcdltdc.pwidth != 0)
		LTDC_Switch(1);
}	 


//LCD�ر���ʾ
void LCD_DisplayOff(void)
{	   
	if (lcdltdc.pwidth != 0)
		LTDC_Switch(0);
}   


//����
//x,y:����
//POINT_COLOR:�˵����ɫ
void LCD_DrawPoint(uint16_t x, uint16_t y)
{
	if (lcdltdc.pwidth != 0)//�����RGB��
	{
		LTDC_Draw_Point(x, y, POINT_COLOR);
	}
}


//���ٻ���
//x,y:����
//color:��ɫ
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint32_t color)
{	   
	if (lcdltdc.pwidth != 0)//�����RGB��
	{
		LTDC_Draw_Point(x, y, color);
		return;
	}
}	 


//����LCD��ʾ����
//dir:0,������1,����
void LCD_Display_Dir(uint8_t dir)
{
    lcddev.dir = dir;         //����/����
    
	if (lcdltdc.pwidth != 0)   //�����RGB��
	{
		LTDC_Display_Dir(dir);
		
		lcddev.width = lcdltdc.width;
		lcddev.height = lcdltdc.height;
		
		return;
	}
}	 

        
//��ʼ��lcd
//�ó�ʼ���������Գ�ʼ�������ͺŵ�LCD(�����.c�ļ���ǰ�������)
void LCD_Init(void)
{ 	  
    lcddev.id = LTDC_PanelID_Read();	//����Ƿ���RGB������
	if (lcddev.id != 0)
	{
		LTDC_Init();			    //ID����,˵����RGB������.
	}

	LCD_Display_Dir(0);		//Ĭ��Ϊ����
	LCD_LED(1);				//��������
	LCD_Clear(WHITE);
}  


//��������
//color:Ҫ���������ɫ
void LCD_Clear(uint32_t color)
{
	uint32_t index = 0;      
	uint32_t totalpoint = lcddev.width;
	
	if (lcdltdc.pwidth != 0)	//�����RGB��
	{
		LTDC_Clear(color);
	}
}  


//��ָ����������䵥����ɫ
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{          
	uint16_t i, j;
	uint16_t xlen = 0;
	
	if (lcdltdc.pwidth != 0)	//�����RGB��
	{
		LTDC_Fill(sx, sy, ex, ey, color);
	}
}  


//��ָ�����������ָ����ɫ��			 
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{  
	uint16_t height, width;
	uint16_t i, j;
	
	if (lcdltdc.pwidth != 0)	//�����RGB��
	{
		LTDC_Color_Fill(sx, sy, ex, ey, color);
	}
}  


//����
//x1,y1:�������
//x2,y2:�յ�����  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	
	int32_t xerr = 0, yerr = 0, delta_x, delta_y, distance; 
	int32_t incx, incy, uRow, uCol; 
	
	delta_x = x2 - x1; //������������ 
	delta_y = y2 - y1; 
	uRow = x1; 
	uCol = y1;
	
	if (delta_x > 0)
		incx = 1; //���õ������� 
	else if (delta_x == 0)
		incx = 0;//��ֱ�� 
	else 
	{
		incx = -1;
		delta_x = -delta_x;
	}
	
	if (delta_y > 0)
		incy = 1; 
	else if (delta_y == 0)
		incy = 0;//ˮƽ�� 
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	
	if (delta_x > delta_y)
		distance = delta_x; //ѡȡ�������������� 
	else 
		distance = delta_y;
	
	for (t = 0; t <= distance + 1; t++ )//������� 
	{  
		LCD_DrawPoint(uRow, uCol);//���� 
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


//������	  
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1, y1, x2, y1);
	LCD_DrawLine(x1, y1, x1, y2);
	LCD_DrawLine(x1, y2, x2, y2);
	LCD_DrawLine(x2, y1, x2, y2);
}


//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r)
{
	int32_t a,b;
	int32_t di;
	
	a = 0;
	b = r;	  
	di = 3 - (r << 1);             //�ж��¸���λ�õı�־

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
		
		//ʹ��Bresenham�㷨��Բ     
		if (di < 0)
			di += 4 * a + 6;	  
		else
		{
			di += 10 + 4 * (a - b);   
			b--;
		} 						    
	}
} 		


//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16/24/32
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode)
{  							  
    uint8_t temp, t1, t;
	uint16_t y0 = y;
	uint8_t csize = (size / 8 + ((size % 8)?1:0)) * (size / 2);		//�õ�����һ���ַ���Ӧ������ռ���ֽ���	
	
 	num = num - ' ';//�õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩
 	
	for (t = 0; t < csize; t++)
	{   
		if (size == 12)
			temp = asc2_1206[num][t]; 	 	//����1206����
		else if (size == 16)
			temp = asc2_1608[num][t];	//����1608����
		else if (size == 24)
			temp = asc2_2412[num][t];	//����2412����
		else if (size == 32)
			temp = asc2_3216[num][t];	//����3216����
		else 
			return;								//û�е��ֿ�

		for (t1 = 0; t1 < 8; t1++)
		{			    
			if (temp & 0x80)
				LCD_Fast_DrawPoint(x, y, POINT_COLOR);
			else if (mode == 0)
				LCD_Fast_DrawPoint(x, y, BACK_COLOR);

			temp <<= 1;
			y++;
			
			if (y >= lcddev.height)
				return;		//��������
			if ((y - y0) == size)
			{
				y = y0;
				x++;
				if (x >= lcddev.width)
					return;	//��������
				break;
			}
		}  	 
	}  	    	   	 	  
}   


//m^n����
//����ֵ:m^n�η�.
uint32_t LCD_Pow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	
	while (n--)
		result *= m;
	
	return result;
}	


//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//color:��ɫ 
//num:��ֵ(0~4294967295);	 
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


//��ʾ����,��λΪ0,������ʾ
//x,y:�������
//num:��ֵ(0~999999999);	 
//len:����(��Ҫ��ʾ��λ��)
//size:�����С
//mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
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


//��ʾ�ַ���
//x,y:�������
//width,height:�����С  
//size:�����С
//*p:�ַ�����ʼ��ַ		  
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p)
{         
	uint8_t x0 = x;
	
	width += x;
	height += y;
	
    while ((*p <= '~') && (*p >= ' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if (x >= width)
		{
			x = x0;
			y += size;
		}
		
        if (y >= height)
			break;//�˳�
			
        LCD_ShowChar(x, y, *p, size, 0);

		x += size / 2;
        p++;
    }  
}


void hal_lcd_init(void)
{
	LCD_Init();
}

