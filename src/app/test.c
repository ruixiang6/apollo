#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include <platform.h>

#define TEST_TASK_STK_SIZE			OSEL_TASK_STACK_SIZE_MAX
#define TEST_TASK_PRIO				OSEL_TASK_PRIO(1)

OSEL_DECLARE_TASK(TEST_TASK, param);

osel_task_t *test_task_h;
osel_event_t *test_event_h;

static uint16_t test_timer_id = 0;

uint8_t ovx_mode=0;							//bit0:0,RGB565模式;1,JPEG模式 
uint16_t curline=0;							//摄像头输出数据,当前行编号
uint16_t yoffset=0;							//y方向的偏移量
#define jpeg_buf_size   30*1024*1024	//定义JPEG数据缓存jpeg_buf的大小(1*4M字节)
#define jpeg_line_size	2*1024			//定义DMA接收数据时,一行数据的最大值

uint32_t dcmi_line_buf[2][jpeg_line_size];	//RGB屏时,摄像头采用一行一行读取,定义行缓存  
const uint8_t *EFFECTS_TBL[7]={"Normal","Cool","Warm","B&W","Yellowish ","Inverse","Greenish"};	//7种特效 

extern void (*dcmi_rx_callback)(void);//DCMI DMA接收回调函数




//定时器测试//////////////////////////////////////////////////////////////////////////////
void test_timeout_cb(void)
{
	DBG_TRACE("test_timeout_cb %d\r\n", test_timer_id);

	hal_timer_free(test_timer_id);
	
	test_timer_id = hal_timer_alloc(1000*1000*4, test_timeout_cb);
}

void timer_test(void)
{
	test_timer_id = hal_timer_alloc(1000*1000*4, test_timeout_cb);
}
////////////////////////////////////////////////////////////////////////////////////////


//LCD LED测试//////////////////////////////////////////////////////////////////////////////
void lcd_led_test(void)
{
	uint8_t key = 0;
	uint8_t x = 0;
	uint8_t lcd_id[12];

	point_color = COLOR_RED; 
	sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);//将LCD ID打印到lcd_id数组。
	
    while (1)
    {
        switch (x) {
		case 0:
			hal_lcd_clear(COLOR_WHITE);
			break;
		case 1:
			hal_lcd_clear(COLOR_BLACK);
			break;
		case 2:
			hal_lcd_clear(COLOR_BLUE);
			break;
		case 3:
			hal_lcd_clear(COLOR_RED);
			break;
		case 4:
			hal_lcd_clear(COLOR_MAGENTA);
			break;
		case 5:
			hal_lcd_clear(COLOR_GREEN);
			break;
		case 6:
			hal_lcd_clear(COLOR_CYAN);
			break; 
		case 7:
			hal_lcd_clear(COLOR_YELLOW);
			break;
		case 8:
			hal_lcd_clear(COLOR_BRRED);
			break;
		case 9:
			hal_lcd_clear(COLOR_GRAY);
			break;
		case 10:
			hal_lcd_clear(COLOR_LGRAY);
			break;
		case 11:
			hal_lcd_clear(COLOR_BROWN);
			break;
		}
		
		point_color = COLOR_RED;
		
		hal_lcd_show_string(10, 40, 260, 32, 32, "China ShangHai"); 	
		hal_lcd_show_string(10, 80, 240, 24, 24, "JiaDing");
		hal_lcd_show_string(10, 110, 240, 16, 16, "WSND");
 		hal_lcd_show_string(10, 130, 240, 16, 16, lcd_id);		//显示LCD ID	      					 
		hal_lcd_show_string(10, 150, 240, 12, 12, "2016/12/22");	
		
	    x++;
		
		if (x == 12)
			x = 0;
		
		hal_led_toggle(LED0);
		hal_led_toggle(LED1);
		
		OSTimeDly(2000);	
	}

}
////////////////////////////////////////////////////////////////////////////////////////


//摄像头测试//////////////////////////////////////////////////////////////////////////////
//RGB屏数据接收回调函数
void rgblcd_dcmi_rx_callback(void)
{  
	uint16_t *pbuf;
	
	if (DMA2_Stream1->CR&(1<<19))//DMA使用buf1,读取buf0
	{ 
		pbuf = (uint16_t*)dcmi_line_buf[0]; 
	}
	else 						//DMA使用buf0,读取buf1
	{
		pbuf = (uint16_t*)dcmi_line_buf[1]; 
	}
	
	hal_lcd_color_fill(0,curline,lcddev.width-1,curline,pbuf);//DM2D填充 


	if (curline < 800 + 112)
	{
		curline++;
	}
	if (curline == 800 + 112)
	{
		curline = 112;
	}

/*
	if (curline < lcddev.height)
	{
		curline++;
	}

	if (curline == lcddev.height)
	{
		curline = 0;
	}
	*/
}

//RGB565测试
//RGB数据直接显示在LCD上面
void rgb565_test(void)
{ 
	uint8_t key;
	uint8_t effect = 0, contrast = 2, fac;
	uint8_t scale = 1;		//默认是全尺寸缩放
	uint8_t msgbuf[15];	//消息缓存区 
	uint16_t outputheight = 0;
	
	hal_lcd_clear(COLOR_WHITE);
	point_color = COLOR_RED; 
	hal_lcd_show_string(30,50,200,16,16,"Apollo STM32F4/F7");
	hal_lcd_show_string(30,70,200,16,16,"OV5640 RGB565 Mode"); 
	hal_lcd_show_string(30,100,200,16,16,"KEY0:Contrast");			//对比度
	hal_lcd_show_string(30,120,200,16,16,"KEY1:Auto Focus"); 		//执行自动对焦
	hal_lcd_show_string(30,140,200,16,16,"KEY2:Effects"); 			//特效 
	hal_lcd_show_string(30,160,200,16,16,"KEY_UP:FullSize/Scale");	//1:1尺寸(显示真实尺寸)/全尺寸缩放
	//自动对焦初始化
	OV5640_RGB565_Mode();	//RGB565模式	
	OV5640_Focus_Init();
	OV5640_Light_Mode(0);	//自动模式
	OV5640_Color_Saturation(3);//色彩饱和度0
	OV5640_Brightness(4);	//亮度0
	OV5640_Contrast(3);		//对比度0
	OV5640_Sharpness(33);	//自动锐度
	OV5640_Focus_Constant();//启动持续对焦
	DCMI_Init();			//DCMI配置

	dcmi_rx_callback=rgblcd_dcmi_rx_callback;//RGB屏接收数据回调函数
    DCMI_DMA_Init((uint32_t)dcmi_line_buf[0],(uint32_t)dcmi_line_buf[1],lcddev.width/2,DMA_MDATAALIGN_HALFWORD,DMA_MINC_ENABLE);

	if(lcddev.height>800)
	{
		yoffset=(lcddev.height-800)/2;
		outputheight=800;
		OV5640_WR_Reg(0x3035,0X51);//降低输出帧率，否则可能抖动
	}else 
	{
		yoffset=0;
		outputheight=lcddev.height;
	}

	
	curline=yoffset;		//行数复位

	//curline = 0;
	
	OV5640_OutSize_Set(4,0,lcddev.width,outputheight);		//满屏缩放显示
	DCMI_Start(); 			//启动传输
	hal_lcd_clear(COLOR_BLACK);
	
	while(1)
	{ 
		key = hal_key_scan(0); 
		if (key)
		{ 
			if (key != KEY1_PRES)
				DCMI_Stop(); //非KEY1按下,停止显示

			switch (key)
			{				    
				case KEY0_PRES:	//对比度设置
					contrast++;
					if(contrast>6)contrast=0;
					OV5640_Contrast(contrast);
					sprintf((char*)msgbuf,"Contrast:%d",(signed char)contrast-3);
					break;
				case KEY1_PRES:	//执行一次自动对焦
					OV5640_Focus_Single();
					break;
				case KEY2_PRES:	//特效设置				 
					effect++;
					if(effect>6)effect=0;
					OV5640_Special_Effects(effect);//设置特效
					sprintf((char*)msgbuf,"%s",EFFECTS_TBL[effect]);
					break;
				case WKUP_PRES:	//1:1尺寸(显示真实尺寸)/缩放	    
					scale=!scale;  
					if(scale==0)
					{
						fac=800/outputheight;	//得到比例因子
 						OV5640_OutSize_Set((1280-fac*lcddev.width)/2,(800-fac*outputheight)/2,lcddev.width,outputheight); 	 
						sprintf((char*)msgbuf,"Full Size 1:1");
					}else 
					{
						OV5640_OutSize_Set(4,0,lcddev.width,outputheight);
 						sprintf((char*)msgbuf,"Scale");
					}
					break;
			}
			
			if(key != KEY1_PRES)	//非KEY1按下
			{
				hal_lcd_show_string(30,50,210,16,16,msgbuf);//显示提示内容
				delay_ms(800); 
				DCMI_Start();	//重新开始传输
			}
		} 
		delay_ms(10);
		//osel_systick_delay(20);
		
	}    
} 

void video_test(void)
{
	point_color = COLOR_RED; 
	
	hal_lcd_show_string(30,50,200,16,16,"Apollo STM32F4/F7");	
	hal_lcd_show_string(30,70,200,16,16,"OV5640 TEST");	
	hal_lcd_show_string(30,90,200,16,16,"ATOM@ALIENTEK");
	hal_lcd_show_string(30,110,200,16,16,"2016/12/22"); 
	
	while (OV5640_Init())//初始化OV5640
	{
		hal_lcd_show_string(30,130,240,16,16,"OV5640 ERR");
		delay_ms(200);
	    hal_lcd_fill(30,130,239,170, COLOR_WHITE);
		delay_ms(200);
        hal_led_toggle(LED0);
	}	
    hal_lcd_show_string(30,130,200,16,16,"OV5640 OK");  
	
	rgb565_test(); 
}
////////////////////////////////////////////////////////////////////////////////////////




//SD测试//////////////////////////////////////////////////////////////////////////////
//通过串口打印SD卡相关信息
void show_sdcard_info(void)
{
	switch (SDCardInfo.CardType) {
	case STD_CAPACITY_SD_CARD_V1_1:
		DBG_PRINTF("Card Type:SDSC V1.1\r\n");break;
	case STD_CAPACITY_SD_CARD_V2_0:
		DBG_PRINTF("Card Type:SDSC V2.0\r\n");break;
	case HIGH_CAPACITY_SD_CARD:
		DBG_PRINTF("Card Type:SDHC V2.0\r\n");break;
	case MULTIMEDIA_CARD:
		DBG_PRINTF("Card Type:MMC Card\r\n");break;
	}
	
  	DBG_PRINTF("Card ManufacturerID:%d\r\n", SDCardInfo.SD_cid.ManufacturerID);	//制造商ID
 	DBG_PRINTF("Card RCA:%d\r\n", SDCardInfo.RCA);								//卡相对地址
	DBG_PRINTF("Card Capacity:%d MB\r\n",(uint32_t)(SDCardInfo.CardCapacity>>20));	//显示容量
 	DBG_PRINTF("Card BlockSize:%d\r\n\r\n", SDCardInfo.CardBlockSize);			//显示块大小
}

//测试SD卡的读取
//从secaddr地址开始,读取seccnt个扇区的数据
//secaddr:扇区地址
//seccnt:扇区数
void sd_test_read(uint32_t secaddr, uint32_t seccnt)
{
	uint32_t i; 
	uint8_t *buf; 
	uint8_t sta = 0;
	
	buf = heap_alloc(seccnt*512, 0);	//申请内存
	
	sta = hal_sdmmc_read_disk(buf, secaddr, seccnt);//读取secaddr扇区开始的内容
	if (sta == 0)						
	{	 
		DBG_PRINTF("SECTOR %d DATA:\r\n", secaddr);
		for (i = 0; i < seccnt*512; i++)
			DBG_PRINTF("%x ", buf[i]);//打印secaddr开始的扇区数据    	   
		DBG_PRINTF("\r\nDATA ENDED\r\n"); 
	}
	else 
		DBG_PRINTF("err:%d\r\n",sta);

	//myfree(SRAMEX,buf);	//释放内存	   
}

//测试SD卡的写入(慎用,最好写全是0XFF的扇区,否则可能损坏SD卡.)
//从secaddr地址开始,写入seccnt个扇区的数据
//secaddr:扇区地址
//seccnt:扇区数
void sd_test_write(uint32_t secaddr, uint32_t seccnt)
{
	uint32_t i;
	uint8_t *buf; 
	uint8_t sta = 0; 
	
	buf = heap_alloc(seccnt*512, 0);	//申请内存
	
	for (i = 0; i < seccnt*512; i++) 
		buf[i]=i*3; 		//初始化写入的数据,是3的倍数.
	sta = hal_sdmmc_write_disk(buf,secaddr,seccnt);		//从secaddr扇区开始写入seccnt个扇区内容

	if (sta == 0) 
		DBG_PRINTF("Write over!\r\n");		  
    else 
		DBG_PRINTF("err:%d\r\n",sta);
	
	//myfree(SRAMEX,buf);					//释放内存	   
}

void sd_test(void)
{
	uint16_t i;
	uint8_t *buf;
	uint32_t sd_size;
	uint8_t wbuf[512];
	uint8_t key;
	
	point_color = COLOR_RED; 

	hal_lcd_show_string(30,50,200,16,16,"Aopllo STM32F4/F7");	
	hal_lcd_show_string(30,70,200,16,16,"SD CARD TEST");	
	hal_lcd_show_string(30,90,200,16,16,"ATOM@ALIENTEK");
	hal_lcd_show_string(30,110,200,16,16,"2016/7/15");   
	hal_lcd_show_string(30,130,200,16,16,"KEY0:Read Sector 0");	   

	while(hal_sdmmc_init())//检测不到SD卡
	{
		hal_lcd_show_string(30,150,200,16,16,"SD Card Error!");
		delay_ms(500);					
		hal_lcd_show_string(30,150,200,16,16,"Please Check! ");
		delay_ms(500);
	}
	
	show_sdcard_info();	//打印SD卡相关信息
 	point_color = COLOR_BLUE;	//设置字体为蓝色 
 	
	//检测SD卡成功 											    
	hal_lcd_show_string(30,150,200,16,16,"SD Card OK    ");
	hal_lcd_show_string(30,170,200,16,16,"SD Card Size:     MB");
	hal_lcd_show_num(30+13*8,170,SDCardInfo.CardCapacity>>20,5,16);//显示SD卡容量	

	for (i = 0; i < 512; i++)
		wbuf[i] = i;

	hal_sdmmc_write_disk(wbuf, 0 , 1);

	DBG_PRINTF("hal_sdmmc_write_disk ok\n");
		
    while (1)
    {
		key = hal_key_scan(0);
		if (key == KEY0_PRES)//KEY0按下了
		{
			buf = heap_alloc(512, 0);		//申请内存
			if (hal_sdmmc_read_disk(buf, 0, 1) == 0)	//读取0扇区的内容
			{	
				hal_lcd_show_string(30,190,200,16,16,"USART1 Sending Data...");
				DBG_PRINTF("SECTOR 0 DATA:\r\n");
				for (sd_size = 0; sd_size < 512; sd_size++)
					DBG_PRINTF("%x ",buf[sd_size]);//打印0扇区数据    	   
				DBG_PRINTF("\r\nDATA ENDED\r\n");
				hal_lcd_show_string(30,190,200,16,16,"USART1 Send Data Over!");
			}
			
			//myfree(0,buf);//释放内存	   
		}  

    }
}


////////////////////////////////////////////////////////////////////////////////////////



OSEL_DECLARE_TASK(TEST_TASK, param)
{
    (void)param;
	osel_event_res_t res;

	DBG_TRACE("TEST_TASK!\r\n");
	

}


void test_init(void)
{	
	/*创建 TEST 任务 */   
	test_task_h = osel_task_create(TEST_TASK, 
								NULL, 
								TEST_TASK_STK_SIZE, 
								TEST_TASK_PRIO);
	DBG_ASSERT(test_task_h != PLAT_NULL);
	test_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(test_event_h != PLAT_NULL);	
}

