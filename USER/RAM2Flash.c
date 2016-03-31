#include "stm32f10x.h"
#include "fsmc_nand.h"
#include <stdio.h>
#define data_length 5000
extern uint16_t data_a[data_length];
void Delay_(__IO uint32_t nCount);

/* Private typedef -----------------------------------------------------------*/
/*����III��LED����ض���*/
#define RCC_GPIO_LED                    RCC_APB2Periph_GPIOF    /*LEDʹ�õ�GPIOʱ��*/
#define LEDn                            4                       /*����III��LED����*/
#define GPIO_LED                        GPIOF                   /*����III��LED��ʹ�õ�GPIO��*/

#define DS1_PIN                         GPIO_Pin_6              /*DS1ʹ�õ�GPIO�ܽ�*/
#define DS2_PIN                         GPIO_Pin_7				/*DS2ʹ�õ�GPIO�ܽ�*/
#define DS3_PIN                         GPIO_Pin_8  			/*DS3ʹ�õ�GPIO�ܽ�*/
//#define DS4_PIN                         GPIO_Pin_9				/*DS4ʹ�õ�GPIO�ܽ�*/
/* Private define ------------------------------------------------------------*/
#define BUFFER_SIZE         0x400
#define NAND_HY_MakerID     0xAD
#define NAND_HY_DeviceID    0xF1
//#define NAND_ST_MakerID     0x20
//#define NAND_ST_DeviceID    0x76

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

NAND_IDTypeDef NAND_ID;
GPIO_InitTypeDef GPIO_InitStructure;
NAND_ADDRESS WriteReadAddr;
USART_InitTypeDef USART_InitStructure;

uint16_t TxBuffer[BUFFER_SIZE], RxBuffer[BUFFER_SIZE];
__IO uint32_t PageNumber = 2, WriteReadStatus = 0, status= 0;
uint32_t j = 0;


/* Private function prototypes -----------------------------------------------*/
void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLenght, uint32_t Offset);

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

void STM32_Shenzhou_COMInit(USART_InitTypeDef* USART_InitStruct)
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

  /* USART configuration */
  USART_Init(USART1, USART_InitStruct);
    
  /* Enable USART */
  USART_Cmd(USART1, ENABLE);
}

void main_ram2flash(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     

  /* Initialize Leds mounted on STM3210X-EVAL board */
  RCC_APB2PeriphClockCmd(RCC_GPIO_LED, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = DS1_PIN | DS2_PIN | DS3_PIN ;//| DS4_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIO_LED, &GPIO_InitStructure);
  GPIO_SetBits(GPIO_LED, DS1_PIN | DS2_PIN | DS3_PIN );//| DS4_PIN);   
  /* USARTx configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  STM32_Shenzhou_COMInit(&USART_InitStructure);
  /* Output a message on Hyperterminal using printf function */
  printf("\n\r--------------------------------------------- ");
  printf("\n\r����III�� Nand Flash��д����");
  printf("\n\r   --DS1��˸��ʾ����III����������");
  printf("\n\r   --DS2--������ʾ��дNand Flash�ɹ�");
  printf("\n\r   --DS3--������ʾ��дNand Flashʧ��");
  printf("\n\r   --DS4--������ʾû�ж���Nand Flash��ID");
  printf("\n\r--------------------------------------------- ");


  /*ʹ��FSMCʱ�� */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

  /*������SRAM���ӵ�FSMC BANK2 NAND*/
  NAND_Init();

  /*��ȡNand Flash ID����ӡ*/
  NAND_ReadID(&NAND_ID);
  printf("\n\r Nand Flash ID:0x%x\t 0x%x\t 0x%x\t 0x%x",NAND_ID.Maker_ID,NAND_ID.Device_ID,
  	                                                    NAND_ID.Third_ID,NAND_ID.Fourth_ID);

  /*У��Nand Flash ��ID�Ƿ���ȷ*/
  if((NAND_ID.Maker_ID == NAND_HY_MakerID) && (NAND_ID.Device_ID == NAND_HY_DeviceID))  
  {

	/*����NAND FLASH��д��ַ*/
    WriteReadAddr.Zone = 0x00;
    WriteReadAddr.Block = 0x00;
    WriteReadAddr.Page = 0x00;
////////////////////////////////////////////////////
	//////////////////////////////////
	////////////////////////
	////nand flash ��������8λ��.
	/*������д�����ݵĿ�*/
    status = NAND_EraseBlock(WriteReadAddr);

    /*��дNand Flash������BUFFER���Ϊ��0x25��ʼ������������һ������ */
    //Fill_Buffer(TxBuffer, BUFFER_SIZE , 0x25);
    /*������д�뵽Nand Flash�С�WriteReadAddr:��д����ʼ��ַ*/	
    	TxBuffer=data_a;
    status = NAND_WriteSmallPage(TxBuffer, WriteReadAddr, PageNumber);

    /*��Nand Flash�ж��ظ�д������ݡ��WriteReadAddr:��д����ʼ��ַ*/
    status = NAND_ReadSmallPage (RxBuffer, WriteReadAddr, PageNumber);
  
    /*�ж϶��ص�������д��������Ƿ�һ��*/  
    for(j = 0; j < BUFFER_SIZE; j++)
    {
      if(TxBuffer[j] != RxBuffer[j])
      {
        WriteReadStatus++;
      }
    }
    printf("\n\r Nand Flash��д���ʳ������н��: ");
    if (WriteReadStatus == 0)
    { 
   	  printf("\n\r Nand Flash��д���ʳɹ�");
      GPIO_ResetBits(GPIO_LED, DS2_PIN);	  
    }
    else
    { 
   	  printf("\n\r Nand Flash��д����ʧ��");	  
	  printf("0x%x",WriteReadStatus);
    
      GPIO_ResetBits(GPIO_LED, DS3_PIN); 	  
	  
    }
  }
  else
  {
   	  printf("\n\r û�м�⵽Nand Flash��ID");	  
      //GPIO_ResetBits(GPIO_LED, DS4_PIN); 
  }

  while(1)
  {
    	GPIO_ResetBits(GPIO_LED, DS1_PIN);
		Delay_(0x3FFFFF);
  		GPIO_SetBits(GPIO_LED, DS1_PIN);
		Delay_(0x3FFFFF);	
  }
}

/**
  *   Function name : Fill_Buffer
  * @brief  Fill the buffer
  * @param  pBuffer: pointer on the Buffer to fill
  * @param  BufferSize: size of the buffer to fill
  * @param  Offset: first value to fill on the Buffer
  */
void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLenght, uint32_t Offset)
{
  uint16_t IndexTmp = 0;

  /* Put in global buffer same values */
  for (IndexTmp = 0; IndexTmp < BufferLenght; IndexTmp++ )
  {
    pBuffer[IndexTmp] = IndexTmp + Offset;
  }
}

PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {
  }

  return ch;
}
