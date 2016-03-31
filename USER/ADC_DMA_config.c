#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include <stdio.h>
#include "ff.h"
#include "stm32_eval_sdio_sd.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h" 


extern u16 ADCConvertedValueLocal;

#define DR_ADDRESS                  ((uint32_t)0x4001244C) //ADC1 DR�Ĵ�������ַ



ADC_InitTypeDef ADC_InitStructure;        //ADC��ʼ���ṹ������
DMA_InitTypeDef DMA_InitStructure;        //DMA��ʼ���ṹ������
USART_InitTypeDef USART_InitStructure;   //���ڳ�ʼ���ṹ������
__IO uint16_t ADCConvertedValue;     // ADCΪ12λģ��ת������ֻ��ADCConvertedValue�ĵ�12λ��Ч


void ADC_GPIO_Configuration(void);

//void STM32_COMInit(USART_InitTypeDef* USART_InitStruct)
	
void COM_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

  /* Enable UART clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 

  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USART Rx as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
  /* USART configuration */
  USART_Init(USART1,&USART_InitStructure);
    
  /* Enable USART */
  USART_Cmd(USART1, ENABLE);
}


void ADC_GPIO_Configuration(void)             //ADC���ú���
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure PC.00 (ADC Channel10) as analog input -------------------------*/
  //PC0 ��Ϊģ��ͨ��10��������                         
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;     //�ܽ�1
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//����ģʽ
  GPIO_Init(GPIOC, &GPIO_InitStructure);     //GPIO��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);     //ʹ��DMAʱ��
  
  /* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);      //����DMA1�ĵ�һͨ��
  DMA_InitStructure.DMA_PeripheralBaseAddr = DR_ADDRESS;      //DMA��Ӧ���������ַ
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValueLocal;   //�ڴ�洢����ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //DMA��ת��ģʽΪSRCģʽ����������Ƶ��ڴ�
  DMA_InitStructure.DMA_BufferSize = 1;      //DMA�����С��1��
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //����һ�����ݺ��豸��ַ��ֹ����
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; //�رս���һ�����ݺ�Ŀ���ڴ��ַ����
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //�����������ݿ��Ϊ16λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  //DMA�������ݳߴ磬HalfWord����Ϊ16λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //ת��ģʽ��ѭ������ģʽ��
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA���ȼ���
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;      //M2Mģʽ����
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);  
  DMA_Cmd(DMA1_Channel1, ENABLE);        
  /* Enable DMA1 channel1 */
  
  
  /* Enable ADC1 and GPIOC clock */
	
  RCC_ADCCLKConfig(RCC_PCLK2_Div8);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);   //ʹ��ADC��GPIOCʱ��
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;    //������ת��ģʽ
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;      //����ɨ��ģʽ
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;   //��������ת��ģʽ
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //ADC�ⲿ���أ��ر�״̬
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   //���뷽ʽ,ADCΪ12λ�У��Ҷ��뷽ʽ
  ADC_InitStructure.ADC_NbrOfChannel = 1;  //����ͨ������1��
  ADC_Init(ADC1, &ADC_InitStructure);
  /* ADC1 regular channel13 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_55Cycles5);
                          //ADCͨ���飬 ��13��ͨ�� ����˳��1��ת��ʱ�� 
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);   //ADC���ʹ��
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);  //����ADC1
  
  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);   //����У׼
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));  //�ȴ�����У׼���
  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);   //��ʼУ׼
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));     //�ȴ�У׼���
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE); //����ת����ʼ��ADCͨ��DMA��ʽ���ϵĸ���RAM����
  //DMA_Cmd(DMA1_Channel1, ENABLE);  //����DMA
}


  