#ifndef __HAL_CAMERA_H
#define __HAL_CAMERA_H


//IO��������
#define SCCB_SDA_IN()  {GPIOB->MODER&=~(3<<(3*2));GPIOB->MODER|=0<<3*2;}	//PB3����ģʽ
#define SCCB_SDA_OUT() {GPIOB->MODER&=~(3<<(3*2));GPIOB->MODER|=1<<3*2;}    //PB3���ģʽ
//IO����
#define SCCB_SCL(n)  (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET)) //SCL
#define SCCB_SDA(n)  (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET)) //SDA

#define SCCB_READ_SDA    HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)     //����SDA
#define SCCB_ID         0X60                                    //OV2640��ID


#define OV5640_RST(n)  	(n?HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET))//��λ�����ź� 
////////////////////////////////////////////////////////////////////////////////// 
#define OV5640_ID               0X5640  	//OV5640��оƬID
 

#define OV5640_ADDR        		0X78		//OV5640��IIC��ַ
 
//OV5640��ؼĴ�������  
#define OV5640_CHIPIDH          0X300A  	//OV5640оƬID���ֽ�
#define OV5640_CHIPIDL          0X300B  	//OV5640оƬID���ֽ�


extern void hal_camera_init(void);

#endif
