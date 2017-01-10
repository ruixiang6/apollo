#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include <platform.h>

#define TEST_TASK_STK_SIZE			OSEL_TASK_STACK_SIZE_MAX
#define TEST_TASK_PRIO				OSEL_TASK_PRIO(1)

OSEL_DECLARE_TASK(TEST_TASK, param);

osel_task_t *test_task_h;
osel_event_t *test_event_h;

static uint16_t test_timer_id = 0;

uint8_t ovx_mode=0;							//bit0:0,RGB565ģʽ;1,JPEGģʽ 
uint16_t curline=0;							//����ͷ�������,��ǰ�б��
uint16_t yoffset=0;							//y�����ƫ����
#define jpeg_buf_size   30*1024*1024	//����JPEG���ݻ���jpeg_buf�Ĵ�С(1*4M�ֽ�)
#define jpeg_line_size	2*1024			//����DMA��������ʱ,һ�����ݵ����ֵ

uint32_t dcmi_line_buf[2][jpeg_line_size];	//RGB��ʱ,����ͷ����һ��һ�ж�ȡ,�����л���  
const uint8_t *EFFECTS_TBL[7]={"Normal","Cool","Warm","B&W","Yellowish ","Inverse","Greenish"};	//7����Ч 

extern void (*dcmi_rx_callback)(void);//DCMI DMA���ջص�����




//��ʱ������//////////////////////////////////////////////////////////////////////////////
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


//LCD LED����//////////////////////////////////////////////////////////////////////////////
void lcd_led_test(void)
{
	uint8_t key = 0;
	uint8_t x = 0;
	uint8_t lcd_id[12];

	point_color = COLOR_RED; 
	sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);//��LCD ID��ӡ��lcd_id���顣
	
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
 		hal_lcd_show_string(10, 130, 240, 16, 16, lcd_id);		//��ʾLCD ID	      					 
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


//����ͷ����//////////////////////////////////////////////////////////////////////////////
//RGB�����ݽ��ջص�����
void rgblcd_dcmi_rx_callback(void)
{  
	uint16_t *pbuf;
	
	if (DMA2_Stream1->CR&(1<<19))//DMAʹ��buf1,��ȡbuf0
	{ 
		pbuf = (uint16_t*)dcmi_line_buf[0]; 
	}
	else 						//DMAʹ��buf0,��ȡbuf1
	{
		pbuf = (uint16_t*)dcmi_line_buf[1]; 
	}
	
	hal_lcd_color_fill(0,curline,lcddev.width-1,curline,pbuf);//DM2D��� 


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

//RGB565����
//RGB����ֱ����ʾ��LCD����
void rgb565_test(void)
{ 
	uint8_t key;
	uint8_t effect = 0, contrast = 2, fac;
	uint8_t scale = 1;		//Ĭ����ȫ�ߴ�����
	uint8_t msgbuf[15];	//��Ϣ������ 
	uint16_t outputheight = 0;
	
	hal_lcd_clear(COLOR_WHITE);
	point_color = COLOR_RED; 
	hal_lcd_show_string(30,50,200,16,16,"Apollo STM32F4/F7");
	hal_lcd_show_string(30,70,200,16,16,"OV5640 RGB565 Mode"); 
	hal_lcd_show_string(30,100,200,16,16,"KEY0:Contrast");			//�Աȶ�
	hal_lcd_show_string(30,120,200,16,16,"KEY1:Auto Focus"); 		//ִ���Զ��Խ�
	hal_lcd_show_string(30,140,200,16,16,"KEY2:Effects"); 			//��Ч 
	hal_lcd_show_string(30,160,200,16,16,"KEY_UP:FullSize/Scale");	//1:1�ߴ�(��ʾ��ʵ�ߴ�)/ȫ�ߴ�����
	//�Զ��Խ���ʼ��
	OV5640_RGB565_Mode();	//RGB565ģʽ	
	OV5640_Focus_Init();
	OV5640_Light_Mode(0);	//�Զ�ģʽ
	OV5640_Color_Saturation(3);//ɫ�ʱ��Ͷ�0
	OV5640_Brightness(4);	//����0
	OV5640_Contrast(3);		//�Աȶ�0
	OV5640_Sharpness(33);	//�Զ����
	OV5640_Focus_Constant();//���������Խ�
	DCMI_Init();			//DCMI����

	dcmi_rx_callback=rgblcd_dcmi_rx_callback;//RGB���������ݻص�����
    DCMI_DMA_Init((uint32_t)dcmi_line_buf[0],(uint32_t)dcmi_line_buf[1],lcddev.width/2,DMA_MDATAALIGN_HALFWORD,DMA_MINC_ENABLE);

	if(lcddev.height>800)
	{
		yoffset=(lcddev.height-800)/2;
		outputheight=800;
		OV5640_WR_Reg(0x3035,0X51);//�������֡�ʣ�������ܶ���
	}else 
	{
		yoffset=0;
		outputheight=lcddev.height;
	}

	
	curline=yoffset;		//������λ

	//curline = 0;
	
	OV5640_OutSize_Set(4,0,lcddev.width,outputheight);		//����������ʾ
	DCMI_Start(); 			//��������
	hal_lcd_clear(COLOR_BLACK);
	
	while(1)
	{ 
		key = hal_key_scan(0); 
		if (key)
		{ 
			if (key != KEY1_PRES)
				DCMI_Stop(); //��KEY1����,ֹͣ��ʾ

			switch (key)
			{				    
				case KEY0_PRES:	//�Աȶ�����
					contrast++;
					if(contrast>6)contrast=0;
					OV5640_Contrast(contrast);
					sprintf((char*)msgbuf,"Contrast:%d",(signed char)contrast-3);
					break;
				case KEY1_PRES:	//ִ��һ���Զ��Խ�
					OV5640_Focus_Single();
					break;
				case KEY2_PRES:	//��Ч����				 
					effect++;
					if(effect>6)effect=0;
					OV5640_Special_Effects(effect);//������Ч
					sprintf((char*)msgbuf,"%s",EFFECTS_TBL[effect]);
					break;
				case WKUP_PRES:	//1:1�ߴ�(��ʾ��ʵ�ߴ�)/����	    
					scale=!scale;  
					if(scale==0)
					{
						fac=800/outputheight;	//�õ���������
 						OV5640_OutSize_Set((1280-fac*lcddev.width)/2,(800-fac*outputheight)/2,lcddev.width,outputheight); 	 
						sprintf((char*)msgbuf,"Full Size 1:1");
					}else 
					{
						OV5640_OutSize_Set(4,0,lcddev.width,outputheight);
 						sprintf((char*)msgbuf,"Scale");
					}
					break;
			}
			
			if(key != KEY1_PRES)	//��KEY1����
			{
				hal_lcd_show_string(30,50,210,16,16,msgbuf);//��ʾ��ʾ����
				delay_ms(800); 
				DCMI_Start();	//���¿�ʼ����
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
	
	while (OV5640_Init())//��ʼ��OV5640
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


//SPIFLASH����//////////////////////////////////////////////////////////////////////////////
const uint8_t SF_TEXT_Buffer[]={"Apo STM32F7 QSPI TETT HAHA"};
#define SFSIZE sizeof(SF_TEXT_Buffer)

void spi_flash_test(void)
{
	uint8_t key;
	uint16_t i=0;
	uint8_t datatemp[SFSIZE];
	uint32_t FLASH_SIZE;

	hal_spi_flash_init();
	
	point_color = COLOR_RED; 
	
	hal_lcd_show_string(30,50,200,16,16,"Apollo STM32F4/F7 hahahahahahahahaaha"); 
    
	hal_lcd_show_string(30,70,200,16,16,"QSPI TEST");	
	hal_lcd_show_string(30,90,200,16,16,"ATOM@ALIENTEK");
	hal_lcd_show_string(30,110,200,16,16,"2016/7/12");	 		
	hal_lcd_show_string(30,130,200,16,16,"KEY1:Write  KEY0:Read");	//��ʾ��ʾ��Ϣ		
	while(hal_spi_flash_readid()!=W25Q256)								//��ⲻ��W25Q256
	{
		hal_lcd_show_string(30,150,200,16,16,"QSPI Check Failed!");
		delay_ms(500);
		hal_lcd_show_string(30,150,200,16,16,"Please Check!        ");
		delay_ms(500);
		hal_led_toggle(0);
	}
	hal_lcd_show_string(30,150,200,16,16,"QSPI Ready!"); 
	FLASH_SIZE=32*1024*1024;	//FLASH ��СΪ32M�ֽ�
  	point_color = COLOR_BLUE;			//��������Ϊ��ɫ	
  	
	while(1)
	{
		key = hal_key_scan(0);
		if(key==KEY1_PRES)//KEY1����,д��W25Q128
		{
			hal_lcd_fill(0,170,239,319,COLOR_WHITE);//�������    
 			hal_lcd_show_string(30,170,200,16,16,"Start Write QSPI....");
			hal_spi_flash_write((u8*)SF_TEXT_Buffer,FLASH_SIZE-100,SFSIZE);		//�ӵ�����100����ַ����ʼ,д��SIZE���ȵ�����
			hal_lcd_show_string(30,170,200,16,16,"QSPI Write Finished!");	//��ʾ�������
		}
		if(key==KEY0_PRES)//KEY0����,��ȡ�ַ�������ʾ
		{
 			hal_lcd_show_string(30,170,200,16,16,"Start Read QSPI.... ");
			hal_spi_flash_read(datatemp,FLASH_SIZE-100,SFSIZE);					//�ӵ�����100����ַ����ʼ,����SIZE���ֽ�
			hal_lcd_show_string(30,170,200,16,16,"The Data Readed Is:   ");	//��ʾ�������
			hal_lcd_show_string(30,190,200,16,16,datatemp);					//��ʾ�������ַ���
		} 
		i++;
		delay_ms(10);
		if(i==20)
		{
			hal_led_toggle(0);
			i=0;
		}		   
	}	   	
}
////////////////////////////////////////////////////////////////////////////////////////


//SD����//////////////////////////////////////////////////////////////////////////////
//ͨ�����ڴ�ӡSD�������Ϣ
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
	
  	DBG_PRINTF("Card ManufacturerID:%d\r\n", SDCardInfo.SD_cid.ManufacturerID);	//������ID
 	DBG_PRINTF("Card RCA:%d\r\n", SDCardInfo.RCA);								//����Ե�ַ
	DBG_PRINTF("Card Capacity:%d MB\r\n",(uint32_t)(SDCardInfo.CardCapacity>>20));	//��ʾ����
 	DBG_PRINTF("Card BlockSize:%d\r\n\r\n", SDCardInfo.CardBlockSize);			//��ʾ���С
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

	while(hal_sdmmc_init())//��ⲻ��SD��
	{
		hal_lcd_show_string(30,150,200,16,16,"SD Card Error!");
		delay_ms(500);					
		hal_lcd_show_string(30,150,200,16,16,"Please Check! ");
		delay_ms(500);
	}
	
	show_sdcard_info();	//��ӡSD�������Ϣ
 	point_color = COLOR_BLUE;	//��������Ϊ��ɫ 
 	
	//���SD���ɹ� 											    
	hal_lcd_show_string(30,150,200,16,16,"SD Card OK    ");
	hal_lcd_show_string(30,170,200,16,16,"SD Card Size:     MB");
	hal_lcd_show_num(30+13*8,170,SDCardInfo.CardCapacity>>20,5,16);//��ʾSD������	

	for (i = 0; i < 512; i++)
		wbuf[i] = i;

	hal_sdmmc_write_disk(wbuf, 0 , 1);

	DBG_PRINTF("hal_sdmmc_write_disk ok\n");
		
    while (1)
    {
		key = hal_key_scan(0);
		if (key == KEY0_PRES)//KEY0������
		{
			buf = heap_alloc(512, 0);		//�����ڴ�
			if (hal_sdmmc_read_disk(buf, 0, 1) == 0)	//��ȡ0����������
			{	
				hal_lcd_show_string(30,190,200,16,16,"USART1 Sending Data...");
				DBG_PRINTF("SECTOR 0 DATA:\r\n");
				for (sd_size = 0; sd_size < 512; sd_size++)
					DBG_PRINTF("%x ",buf[sd_size]);//��ӡ0��������    	   
				DBG_PRINTF("\r\nDATA ENDED\r\n");
				hal_lcd_show_string(30,190,200,16,16,"USART1 Send Data Over!");
			}
			
			//myfree(0,buf);//�ͷ��ڴ�	   
		}  

    }
}
////////////////////////////////////////////////////////////////////////////////////////



//FatFs�ļ�ϵͳ����//////////////////////////////////////////////////////////////////////////////
void fatfs_test(void)
{
	uint32_t total,free;
	uint8_t t=0;	
	uint8_t res=0;	
	
	point_color = COLOR_RED; 
	
	hal_lcd_show_string(30,50,200,16,16,"Apollo STM32F4/F7"); 
	hal_lcd_show_string(30,70,200,16,16,"FATFS TEST");	
	hal_lcd_show_string(30,90,200,16,16,"ATOM@ALIENTEK");
	hal_lcd_show_string(30,110,200,16,16,"2016/7/15");	 	 
	hal_lcd_show_string(30,130,200,16,16,"Use USMART for test");
	
 	while(hal_sdmmc_init())//��ⲻ��SD��
	{
		hal_lcd_show_string(30,150,200,16,16,"SD Card Error!");
		delay_ms(500);					
		hal_lcd_show_string(30,150,200,16,16,"Please Check! ");
		delay_ms(500);
		hal_led_toggle(0);
	}
	
 	exfuns_init();							//Ϊfatfs��ر��������ڴ�	
 	
  	f_mount(fs[0],"0:",1); 					//����SD�� 
  	
	hal_lcd_fill(30,150,240,150+16, COLOR_WHITE);		//�����ʾ		
	
	while(exf_getfree("0:",&total,&free))	//�õ�SD������������ʣ������
	{
		hal_lcd_show_string(30,150,200,16,16,"SD Card Fatfs Error!");
		delay_ms(200);
		hal_lcd_fill(30,150,240,150+16,COLOR_WHITE);	//�����ʾ			  
		delay_ms(200);
		hal_led_toggle(0);
	}	
	
 	point_color = COLOR_BLUE;//��������Ϊ��ɫ	   
 	
	hal_lcd_show_string(30,150,200,16,16,"FATFS OK!");	 
	hal_lcd_show_string(30,170,200,16,16,"SD Total Size:     MB");	 
	hal_lcd_show_string(30,190,200,16,16,"SD  Free Size:     MB"); 	    
 	hal_lcd_show_num(30+8*14,170,total>>10,5,16);	//��ʾSD�������� MB
 	hal_lcd_show_num(30+8*14,190,free>>10,5,16);     //��ʾSD��ʣ������ MB		
 	
	while(1)
	{
		t++; 
		delay_ms(200);		 			   
		hal_led_toggle(0);
	} 

}
////////////////////////////////////////////////////////////////////////////////////////



//text����//////////////////////////////////////////////////////////////////////////////
void text_test(void)
{
	u32 fontcnt;		  
	u8 i,j;
	u8 fontx[2];                    //gbk��
	u8 key,t;	

	  exfuns_init();				  //Ϊfatfs��ر��������ڴ�  
	  f_mount(fs[0],"0:",1);		  //����SD�� 
	  f_mount(fs[1],"1:",1);		  //����SPI FLASH.
	  while(font_init())			  //����ֿ�
	  {
	UPD:	
		  hal_lcd_clear(COLOR_WHITE); 		  //����
		  point_color = COLOR_RED; 
		  
		  hal_lcd_show_string(30,50,200,16,16,"Apollo STM32F4/F7");
		  while(hal_sdmmc_init())			  //���SD��
		  {
			  hal_lcd_show_string(30,70,200,16,16,"SD Card Failed!");
			  delay_ms(200);
			  hal_lcd_fill(30,70,200+30,70+16,COLOR_WHITE);
			  delay_ms(200);		  
		   }														  
		  hal_lcd_show_string(30,70,200,16,16,"SD Card OK");
		  hal_lcd_show_string(30,90,200,16,16,"Font Updating...");
		  key=update_font(20,110,16,"0:");//�����ֿ�
		  while(key)//����ʧ��		  
		  { 	  
			   hal_lcd_show_string(30,110,200,16,16,"Font Update Failed!");
			   delay_ms(200);
			   hal_lcd_fill(20,110,200+20,110+16,COLOR_WHITE);
			   delay_ms(200);			 
		  } 		
		  hal_lcd_show_string(30,110,200,16,16,"Font Update Success!   ");
		  delay_ms(1500); 
		  
		  hal_lcd_clear(COLOR_WHITE); 		  //����
		  		 
	  } 
	  point_color = COLOR_RED; 	 
	  Show_Str(30,30,200,16,"������STM32F4/F7������",16,0); 					   
	  Show_Str(30,50,200,16,"GBK�ֿ���Գ���",16,0);					   
	  Show_Str(30,70,200,16,"����ԭ��@ALIENTEK",16,0);						   
	  Show_Str(30,90,200,16,"2016��7��15��",16,0);
	  Show_Str(30,110,200,16,"��KEY0,�����ֿ�",16,0);
	 point_color = COLOR_BLUE;  
	  Show_Str(30,130,200,16,"������ֽ�:",16,0);					   
	  Show_Str(30,150,200,16,"������ֽ�:",16,0);					   
	  Show_Str(30,170,200,16,"���ּ�����:",16,0);
	
	  Show_Str(30,200,200,32,"��Ӧ����Ϊ:",32,0); 
	  Show_Str(30,232,200,24,"��Ӧ����Ϊ:",24,0); 
	  Show_Str(30,256,200,16,"��Ӧ����(16*16)Ϊ:",16,0);		   
	  Show_Str(30,272,200,12,"��Ӧ����(12*12)Ϊ:",12,0);		   
	  while(1)
	  {
		  fontcnt=0;
		  for(i=0x81;i<0xff;i++)
		  { 	  
			  fontx[0]=i;
			  hal_lcd_show_num(118,150,i,3,16);		  //��ʾ������ֽ�	  
			  for(j=0x40;j<0xfe;j++)
			  {
				  if(j==0x7f)continue;
				  fontcnt++;
				  hal_lcd_show_num(118,150,j,3,16);	  //��ʾ������ֽ�	   
				  hal_lcd_show_num(118,170,fontcnt,5,16);//���ּ�����ʾ	   
				  fontx[1]=j;
				  Show_Font(30+176,200,fontx,32,0);
				  Show_Font(30+132,232,fontx,24,0); 	
				  Show_Font(30+144,256,fontx,16,0); 				   
				  Show_Font(30+108,272,fontx,12,0); 				   
				  t=200;
				  while(t--)//��ʱ,ͬʱɨ�谴��
				  {
					  delay_ms(1);
					  key=hal_key_scan(0);
					  if(key==KEY0_PRES)goto UPD;
				  }
				  hal_led_toggle(0);
			  }   
		  }   
	  } 

}
////////////////////////////////////////////////////////////////////////////////////////

//picture����//////////////////////////////////////////////////////////////////////////////
//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO *tfileinfo;	//��ʱ�ļ���Ϣ	    			     
	tfileinfo=(FILINFO*)heap_alloc(sizeof(FILINFO),1);//�����ڴ�
    res=f_opendir(&tdir,(const TCHAR*)path); 	//��Ŀ¼ 
	if(res==FR_OK&&tfileinfo)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�  	 
	        if(res!=FR_OK||tfileinfo->fname[0]==0)break;//������/��ĩβ��,�˳�	 		 
			res=f_typetell((u8*)tfileinfo->fname);
			if((res&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	}  
	//myfree(SRAMIN,tfileinfo);//�ͷ��ڴ�
	return rval;
}

void picture_test(void)
{
	uint8_t i = 0;
	u8 res;
 	DIR picdir;	 		//ͼƬĿ¼
	FILINFO *picfileinfo;//�ļ���Ϣ 
	u8 *pname[5];			//��·�����ļ���
	u16 totpicnum; 		//ͼƬ�ļ�����
	u16 curindex;		//ͼƬ��ǰ����
	u8 key;				//��ֵ
	u8 pause=0;			//��ͣ���
	u8 t;
	u16 temp;
	u32 *picoffsettbl;	//ͼƬ�ļ�offset������ 
	uint8_t image_file_num = 0;

	exfuns_init();				    //Ϊfatfs��ر��������ڴ�  
 	f_mount(fs[0],"0:",1); 		    //����SD�� 
 	f_mount(fs[1],"1:",1); 		    //����FLASH.
	 point_color = COLOR_RED; 	   
	while(font_init()) 		        //����ֿ�
	{	    
		hal_lcd_show_string(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		hal_lcd_fill(30,50,240,66,COLOR_WHITE);//�����ʾ	     
		delay_ms(200);				  
	}  	 
 	Show_Str(30,50,200,16,"������STM32F4/F7������",16,0);				    	 
	Show_Str(30,70,200,16,"ͼƬ��ʾ����",16,0);				    	 
	Show_Str(30,90,200,16,"KEY0:NEXT KEY2:PREV",16,0);				    	 
	Show_Str(30,110,200,16,"KEY_UP:PAUSE",16,0);				    	 
	Show_Str(30,130,200,16,"����ԭ��@ALIENTEK",16,0);				    	 
	Show_Str(30,150,200,16,"2016��7��15��",16,0);
 	while(f_opendir(&picdir,"0:/PICTURE"))//��ͼƬ�ļ���
 	{	    
		Show_Str(30,170,240,16,"PICTURE�ļ��д���!",16,0);
		delay_ms(200);				  
		hal_lcd_fill(30,170,240,186,COLOR_WHITE);//�����ʾ	     
		delay_ms(200);				  
	}  
	totpicnum=pic_get_tnum("0:/PICTURE"); //�õ�����Ч�ļ���
  	while(totpicnum==NULL)//ͼƬ�ļ�Ϊ0		
 	{	    
		Show_Str(30,170,240,16,"û��ͼƬ�ļ�!",16,0);
		delay_ms(200);				  
		hal_lcd_fill(30,170,240,186,COLOR_WHITE);//�����ʾ	     
		delay_ms(200);				  
	} 

	
	picfileinfo=(FILINFO*)heap_alloc(sizeof(FILINFO),1);	//�����ڴ�

	for (i = 0; i < 5; i++)
	{
	 	pname[i]=heap_alloc(_MAX_LFN*2+1,1);					//Ϊ��·�����ļ��������ڴ�
	}
 	picoffsettbl=heap_alloc(4*totpicnum,1);					//����4*totpicnum���ֽڵ��ڴ�,���ڴ��ͼƬ����
 	while(!picfileinfo||!pname[0]||!picoffsettbl)					//�ڴ�������
 	{	    	
		Show_Str(30,170,240,16,"�ڴ����ʧ��!",16,0);
		delay_ms(200);				  
		hal_lcd_fill(30,170,240,186,COLOR_WHITE);//�����ʾ	     
		delay_ms(200);				  
	}  	
	//��¼����
    res=f_opendir(&picdir,"0:/PICTURE"); //��Ŀ¼
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{
			temp=picdir.dptr;								//��¼��ǰdptrƫ��
	        res=f_readdir(&picdir,picfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||picfileinfo->fname[0]==0)break;	//������/��ĩβ��,�˳�	 	 
			res=f_typetell((u8*)picfileinfo->fname);	
			if((res&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
			{
				picoffsettbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	} 

	image_file_num = curindex;
	
	Show_Str(30,170,240,16,"��ʼ��ʾ...",16,0); 
	delay_ms(1500);
	piclib_init();										//��ʼ����ͼ	   	   
	curindex=0;											//��0��ʼ��ʾ
   	res=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//��Ŀ¼
	for (i = 0; i < image_file_num; i++)
	{
		dir_sdi(&picdir,picoffsettbl[i]);			//�ı䵱ǰĿ¼����	   
        res=f_readdir(&picdir,picfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
        if(res!=FR_OK||picfileinfo->fname[0]==0)break;	//������/��ĩβ��,�˳�
		strcpy((char*)pname[i],"0:/PICTURE/");				//����·��(Ŀ¼)
		strcat((char*)pname[i],(const char*)picfileinfo->fname);//���ļ������ں���		
	}

	
	while(res==FR_OK)//�򿪳ɹ�
	{	
 		hal_lcd_clear(COLOR_BLACK);
 		//ai_load_picfile(pname[curindex],0,0,lcddev.width,lcddev.height,0);//��ʾͼƬ    
 		ai_load_picfile(pname[curindex],0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ  
		//Show_Str(2,2,lcddev.width,16,pname[curindex],16,1); 				//��ʾͼƬ����
		
		curindex++;		   	
		if(curindex>=image_file_num)
		{
			curindex=0;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
			//hal_lcd_clear(COLOR_BLUE);
		}
		
		delay_ms(500);
					    		
	} 	

	return;

}
////////////////////////////////////////////////////////////////////////////////////////


OSEL_DECLARE_TASK(TEST_TASK, param)
{
    (void)param;
	osel_event_res_t res;

	DBG_TRACE("TEST_TASK!\r\n");

	picture_test();
}


void test_init(void)
{	
	/*���� TEST ���� */   
	test_task_h = osel_task_create(TEST_TASK, 
								NULL, 
								TEST_TASK_STK_SIZE, 
								TEST_TASK_PRIO);
	DBG_ASSERT(test_task_h != PLAT_NULL);
	test_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(test_event_h != PLAT_NULL);	
}

