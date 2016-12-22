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

void test_timeout_cb(void)
{
	DBG_TRACE("test_timeout_cb %d\r\n", test_timer_id);

	hal_timer_free(test_timer_id);
	
	test_timer_id = hal_timer_alloc(1000*1000*4, test_timeout_cb);
}






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











OSEL_DECLARE_TASK(TEST_TASK, param)
{
    (void)param;
	osel_event_res_t res;

	uint8_t key = 0;
	uint8_t x = 0;
	uint8_t lcd_id[12];


	DBG_TRACE("TEST_TASK!\r\n");

	//test_timer_id = hal_timer_alloc(1000*1000*4, test_timeout_cb);

	point_color = COLOR_RED; 
	sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);//��LCD ID��ӡ��lcd_id���顣
	
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
		
/*
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
*/	
}
