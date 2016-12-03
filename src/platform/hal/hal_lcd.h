#ifndef __HAL_LCD_H
#define __HAL_LCD_H


#define LCD_PIXEL_FORMAT_ARGB8888       0X00    
#define LCD_PIXEL_FORMAT_RGB888         0X01    
#define LCD_PIXEL_FORMAT_RGB565         0X02       
#define LCD_PIXEL_FORMAT_ARGB1555       0X03      
#define LCD_PIXEL_FORMAT_ARGB4444       0X04     
#define LCD_PIXEL_FORMAT_L8             0X05     
#define LCD_PIXEL_FORMAT_AL44           0X06     
#define LCD_PIXEL_FORMAT_AL88           0X07      

//������ɫ���ظ�ʽ,һ����RGB565
#define LCD_PIXFORMAT				LCD_PIXEL_FORMAT_RGB565	
//����Ĭ�ϱ�������ɫ
#define LTDC_BACKLAYERCOLOR			0X00000000	
//LCD֡�������׵�ַ,���ﶨ����SDRAM����.
#define LCD_FRAME_BUF_ADDR			0XC0000000  


//������ɫ
#define COLOR_WHITE         	 0xFFFF
#define COLOR_BLACK         	 0x0000	  
#define COLOR_BLUE         	 	 0x001F  
#define COLOR_BRED             	 0XF81F
#define COLOR_GRED 			 	 0XFFE0
#define COLOR_GBLUE				 0X07FF
#define COLOR_RED           	 0xF800
#define COLOR_MAGENTA       	 0xF81F
#define COLOR_GREEN         	 0x07E0
#define COLOR_CYAN          	 0x7FFF
#define COLOR_YELLOW        	 0xFFE0
#define COLOR_BROWN 			 0XBC40 //��ɫ
#define COLOR_BRRED 			 0XFC07 //�غ�ɫ
#define COLOR_GRAY  			 0X8430 //��ɫ
//GUI��ɫ

#define COLOR_DARKBLUE      	 0X01CF	//����ɫ
#define COLOR_LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define COLOR_GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define COLOR_LIGHTGREEN     	 0X841F //ǳ��ɫ
#define COLOR_LGRAY 			 0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define COLOR_LGRAYBLUE          0XA651 //ǳ����ɫ(�м����ɫ)
#define COLOR_LBBLUE             0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)


typedef struct _ltdc_dev_t {							 
	uint32_t pwidth;			//LCD���Ŀ��,�̶�����,������ʾ����ı�,���Ϊ0,˵��û���κ�RGB������
	uint32_t pheight;		//LCD���ĸ߶�,�̶�����,������ʾ����ı�
	uint16_t hsw;			//ˮƽͬ�����
	uint16_t vsw;			//��ֱͬ�����
	uint16_t hbp;			//ˮƽ����
	uint16_t vbp;			//��ֱ����
	uint16_t hfp;			//ˮƽǰ��
	uint16_t vfp;			//��ֱǰ�� 
	uint8_t activelayer;		//��ǰ����:0/1	
	uint8_t dir;				//0,����;1,����;
	uint16_t width;			//LCD���
	uint16_t height;			//LCD�߶�
	uint32_t pixsize;		//ÿ��������ռ�ֽ���
} ltdc_dev_t; 


typedef struct _lcd_dev_t {		 	 
	uint16_t width;			//LCD ���
	uint16_t height;			//LCD �߶�
	uint16_t id;				//LCD ID
	uint8_t  dir;			//���������������ƣ�0��������1��������	
	uint16_t	wramcmd;		//��ʼдgramָ��
	uint16_t setxcmd;		//����x����ָ��
	uint16_t setycmd;		//����y����ָ�� 
} lcd_dev_t; 	 



extern uint32_t point_color;
extern lcd_dev_t lcddev;


extern void hal_lcd_led(uint8_t status);
extern void hal_lcd_display_dir(uint8_t dir);
extern uint32_t hal_lcd_read_point(uint16_t x, uint16_t y);
extern void hal_lcd_draw_point(uint16_t x, uint16_t y);
extern void hal_lcd_draw_color_point(uint16_t x, uint16_t y, uint32_t color);
extern void hal_lcd_clear(uint32_t color);
extern void hal_lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);
extern void hal_lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);
extern void hal_lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern void hal_lcd_draw_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern void hal_lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r);
extern void hal_lcd_show_char(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode);
extern void hal_lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size);
extern void hal_lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);
extern void hal_lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p);
extern void hal_lcd_init(void);

#endif
