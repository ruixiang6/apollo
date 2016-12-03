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

//定义颜色像素格式,一般用RGB565
#define LCD_PIXFORMAT				LCD_PIXEL_FORMAT_RGB565	
//定义默认背景层颜色
#define LTDC_BACKLAYERCOLOR			0X00000000	
//LCD帧缓冲区首地址,这里定义在SDRAM里面.
#define LCD_FRAME_BUF_ADDR			0XC0000000  


//画笔颜色
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
#define COLOR_BROWN 			 0XBC40 //棕色
#define COLOR_BRRED 			 0XFC07 //棕红色
#define COLOR_GRAY  			 0X8430 //灰色
//GUI颜色

#define COLOR_DARKBLUE      	 0X01CF	//深蓝色
#define COLOR_LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define COLOR_GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define COLOR_LIGHTGREEN     	 0X841F //浅绿色
#define COLOR_LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define COLOR_LGRAYBLUE          0XA651 //浅灰蓝色(中间层颜色)
#define COLOR_LBBLUE             0X2B12 //浅棕蓝色(选择条目的反色)


typedef struct _ltdc_dev_t {							 
	uint32_t pwidth;			//LCD面板的宽度,固定参数,不随显示方向改变,如果为0,说明没有任何RGB屏接入
	uint32_t pheight;		//LCD面板的高度,固定参数,不随显示方向改变
	uint16_t hsw;			//水平同步宽度
	uint16_t vsw;			//垂直同步宽度
	uint16_t hbp;			//水平后廊
	uint16_t vbp;			//垂直后廊
	uint16_t hfp;			//水平前廊
	uint16_t vfp;			//垂直前廊 
	uint8_t activelayer;		//当前层编号:0/1	
	uint8_t dir;				//0,竖屏;1,横屏;
	uint16_t width;			//LCD宽度
	uint16_t height;			//LCD高度
	uint32_t pixsize;		//每个像素所占字节数
} ltdc_dev_t; 


typedef struct _lcd_dev_t {		 	 
	uint16_t width;			//LCD 宽度
	uint16_t height;			//LCD 高度
	uint16_t id;				//LCD ID
	uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t	wramcmd;		//开始写gram指令
	uint16_t setxcmd;		//设置x坐标指令
	uint16_t setycmd;		//设置y坐标指令 
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
