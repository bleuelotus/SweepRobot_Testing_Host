/**
  ******************************************************************************
  * @file    Usart.c
  * @author  Reason Chen
  * @version V1.0
  * @date    5-May-2015
  * @brief   redefine printf
  ******************************************************************************
  */

#include <stdlib.h>
#include <string.h>
#include "Usart.h"

#define STDIO_UART          USART1

#define UART4_TX_BUF_LEN            100
#define UART4_RX_CMD_BUF_LEN        100
#define UART4_RX_CMD_ACT_BUF_LEN    100

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar()
#else
#ifdef USE_KEIL_MDK
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#elif defined USE_IAR_EWARM
#define PUTCHAR_PROTOTYPE int putchar(int ch)
#define GETCHAR_PROTOTYPE int getchar(void)
#endif
#endif

//static u8 UartTxBuf[UART4_TX_BUF_LEN];
u16 UsartRxState = 0;
static char UsartRxCmdBuf[UART4_RX_CMD_BUF_LEN];
static char UsartRxCmdActBuf[UART4_RX_CMD_ACT_BUF_LEN];
static u16 UsartRxCmdLen, UsartRxCmdActLen;
static int UsartRxSubParaBuf;

static void USART1_RX_ISR(void);
void USART_TestCtrlCmdSend(enum USARTTestCtrlCmd cmd, enum USARTTestCtrlCmdAct cmd_act, int cmd_para);

void USART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* config USART1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_AHB1Periph_GPIOA, ENABLE);

    /* USART1 GPIO config */
    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART1 Rx (PB.7) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void USART1_RX_ISR(void)
{
    u8 ch;
    
    if(USART_GetITStatus(STDIO_UART, USART_IT_RXNE) != RESET){
        ch = USART_ReceiveData(STDIO_UART);
        if( !(UsartRxState & (1<<USART_RX_STATE_REC_FINISH_POS) ) ){
            if( UsartRxState & (1<<USART_RX_STATE_CR_REC_POS) ){
                if(ch != '\n')
                    UsartRxState = 0;
                else{
                    UsartRxState |= (1<<USART_RX_STATE_REC_FINISH_POS);
#ifdef DEBUG_LOG
//                printf("%s->%s=%d\r\n", UsartRxCmdBuf, UsartRxCmdActBuf, UartRxCmdParaBuf);
#endif
                }
            }else{
                if(ch == '\r')
                    UsartRxState |= (1<<USART_RX_STATE_CR_REC_POS);
                else{
                    if( !(UsartRxState & (1<<USART_RX_STATE_CMD_PARA_REC_EN_POS) ) ){
                        if( ch == '='){
                            UsartRxState |= (1<<USART_RX_STATE_CMD_PARA_REC_EN_POS);
                            UsartRxState &= USART_RX_STATE_CNT_CLR_MASK;
                            return;
                        }
                        if( !(UsartRxState & (1<<USART_RX_STATE_CMD_ACT_REC_EN_POS) ) ){
                            if( !(UsartRxState & (1<<USART_RX_STATE_DASH_REC_POS) ) ){
                                if( ch == '-'){
                                    UsartRxState |= (1<<USART_RX_STATE_DASH_REC_POS);
                                    return;
                                }
                                if( (('a' <= ch) && ('z' >= ch)) || (('A' <= ch) && ('Z' >= ch)) || ('_' == ch) ){
                                    UsartRxCmdBuf[UsartRxState&USART_RX_STATE_CNT_MASK] = ch;
                                    UsartRxState++;
                                    UsartRxCmdLen = UsartRxState&USART_RX_STATE_CNT_MASK;
                                    if( (UsartRxState&USART_RX_STATE_CNT_MASK) >  UART4_RX_CMD_BUF_LEN){
                                        UsartRxState = 0;
                                        return;
                                    }
                                }else{
                                    UsartRxState = 0;
                                    return;
                                }
                            }else{
                                if( ch == '>'){
                                    UsartRxState |= (1<<USART_RX_STATE_CMD_ACT_REC_EN_POS);
                                    UsartRxState &= USART_RX_STATE_CNT_CLR_MASK;
                                }else{
                                    UsartRxState = 0;
                                    return;
                                }
                            }
                        }else{
                            if( (('a' <= ch) && ('z' >= ch)) || (('A' <= ch) && ('Z' >= ch)) || ('_' == ch) ){
                                UsartRxCmdActBuf[UsartRxState&USART_RX_STATE_CNT_MASK] = ch;
                                UsartRxState++;
                                UsartRxCmdActLen = UsartRxState&USART_RX_STATE_CNT_MASK;
                                if( (UsartRxState&USART_RX_STATE_CNT_MASK) >  UART4_RX_CMD_BUF_LEN){
                                    UsartRxState = 0;
                                    return;
                                }
                            }else{
                                UsartRxState = 0;
                                return;
                            }
                        }
                    }else{
                        if( ('0' <= ch) && ('9' >= ch) ){
                            if(UsartRxState&USART_RX_STATE_CNT_MASK){
                                UsartRxSubParaBuf *= 10;
                            }
                            UsartRxSubParaBuf += (ch - '0');
                            UsartRxState++;
                        }else{
                            UsartRxState = 0;
                            return;
                        }
                    }
                }
            }
        }
    }
}

s8 USART_CmdArrayToString(char *src_array, char* *dest_str)
{
    *dest_str = (char *)malloc(sizeof(char)*(UsartRxCmdLen+1) );
    
    if(NULL==(*dest_str) )
        return -1;
    
    memset(*dest_str, 0, sizeof(char)*(UsartRxCmdLen+1));
    strncpy(*dest_str, src_array, UsartRxCmdLen);
    
    return 0;
}

s8 USART_CmdActArrayToString(char *src_array, char* *dest_str)
{
    *dest_str = (char *)malloc(sizeof(char)*(UsartRxCmdActLen+1) );
    
    if(NULL==(*dest_str) )
        return -1;
    
    memset(*dest_str, 0, sizeof(char)*(UsartRxCmdActLen+1));
    strncpy(*dest_str, src_array, UsartRxCmdActLen);
    
    return 0;
}

void USART_CmdProcFinishProc(void)
{
    UsartRxState = 0;
    UsartRxSubParaBuf = 0;
}

void USART_CmdProc(void)
{
    enum USARTTestCtrlCmd cmd;
    enum USARTTestCtrlCmdAct cmd_act;
    
    char *UsartRxCmdStrBuf = NULL;
    char *UsartRxCmdActStrBuf = NULL;
    
    if( USART_CmdArrayToString(UsartRxCmdBuf, &UsartRxCmdStrBuf) )
        return;
    if(       !(strcmp(UsartRxCmdStrBuf, "TEST") ) ){
        cmd = TEST_CTRL_CMD_TEST;
    }else if( !(strcmp(UsartRxCmdStrBuf, "LWHEEL") ) ){
        cmd = TEST_CTRL_CMD_LWHEEL;
    }else if( !(strcmp(UsartRxCmdStrBuf, "RWHEEL") ) ){
        cmd = TEST_CTRL_CMD_RWHEEL;
    }else if( !(strcmp(UsartRxCmdStrBuf, "LBRUSH") ) ){
        cmd = TEST_CTRL_CMD_LBRUSH;
    }else if( !(strcmp(UsartRxCmdStrBuf, "RBRUSH") ) ){
        cmd = TEST_CTRL_CMD_RBRUSH;
    }else if( !(strcmp(UsartRxCmdStrBuf, "MBRUSH") ) ){
        cmd = TEST_CTRL_CMD_MBRUSH;
    }else if( !(strcmp(UsartRxCmdStrBuf, "FAN") ) ){
        cmd = TEST_CTRL_CMD_FAN;
    }else if( !(strcmp(UsartRxCmdStrBuf, "SENSOR") ) ){
        cmd = TEST_CTRL_CMD_SENSOR;
    }else if( !(strcmp(UsartRxCmdStrBuf, "RGB_LED") ) ){
        cmd = TEST_CTRL_CMD_RGB_LED;
    }else if( !(strcmp(UsartRxCmdStrBuf, "KEY") ) ){
        cmd = TEST_CTRL_CMD_KEY;
    }else if( !(strcmp(UsartRxCmdStrBuf, "IRDA") ) ){
        cmd = TEST_CTRL_CMD_IRDA;
    }else if( !(strcmp(UsartRxCmdStrBuf, "BUZZER") ) ){
        cmd = TEST_CTRL_CMD_BUZZER;
    }else if( !(strcmp(UsartRxCmdStrBuf, "CHARGE") ) ){
        cmd = TEST_CTRL_CMD_CHARGE;
    }else{
        free(UsartRxCmdStrBuf);
        UsartRxCmdStrBuf = NULL;
        USART_CmdProcFinishProc();
        return;
    }
    free(UsartRxCmdStrBuf);
    UsartRxCmdStrBuf = NULL;
    
    if( USART_CmdActArrayToString(UsartRxCmdActBuf, &UsartRxCmdActStrBuf) )
        return;
    if(       !(strcmp(UsartRxCmdActStrBuf, "ON") ) ){
        cmd_act = TEST_CTRL_CMD_ACT_ON;
    }else if( !(strcmp(UsartRxCmdActStrBuf, "OFF") ) ){
        cmd_act = TEST_CTRL_CMD_ACT_OFF;
    }else if( !(strcmp(UsartRxCmdActStrBuf, "READ") ) ){
        cmd_act = TEST_CTRL_CMD_ACT_READ;
    }else if( !(strcmp(UsartRxCmdActStrBuf, "DIR") ) ){
        cmd_act = TEST_CTRL_CMD_ACT_DIR;
    }else if( !(strcmp(UsartRxCmdActStrBuf, "SPEED") ) ){
        cmd_act = TEST_CTRL_CMD_ACT_SPEED;
    }else if( !(strcmp(UsartRxCmdActStrBuf, "IFRD_LED") ) ){
        cmd_act = TEST_CTRL_CMD_ACT_IFRD_LED;
    }else if( !(strcmp(UsartRxCmdActStrBuf, "B_SWITCH") ) ){
        cmd_act = TEST_CTRL_CMD_ACT_B_SWITCH;
    }else if( !(strcmp(UsartRxCmdActStrBuf, "START") ) ){
        cmd_act = TEST_CTRL_CMD_ACT_START;
    }else if( !(strcmp(UsartRxCmdActStrBuf, "STOP") ) ){
        cmd_act = TEST_CTRL_CMD_ACT_STOP;
    }else{
        free(UsartRxCmdActStrBuf);
        UsartRxCmdActStrBuf = NULL;
        USART_CmdProcFinishProc();
        return;
    }
    free(UsartRxCmdActStrBuf);
    UsartRxCmdActStrBuf = NULL;

    USART_TestCtrlCmdSend(cmd, cmd_act, UsartRxSubParaBuf);
    
    USART_CmdProcFinishProc();
}

PUTCHAR_PROTOTYPE
{
    USART_SendData(STDIO_UART, (u8) ch);

    while (USART_GetFlagStatus(STDIO_UART, USART_FLAG_TXE) == RESET);

    return (ch);
}

GETCHAR_PROTOTYPE
{
    int ch;
    
    while (USART_GetFlagStatus(STDIO_UART, USART_FLAG_RXNE) == RESET);
    ch = USART_ReceiveData(STDIO_UART);
    
//    while (USART_GetFlagStatus(STDIO_UART, USART_FLAG_TC) == RESET);
//    USART_SendData(STDIO_UART, (uint8_t)ch);
    
    return ch;
}


void USART_TestCtrlCmdSend(enum USARTTestCtrlCmd cmd, enum USARTTestCtrlCmdAct cmd_act, int cmd_para)
{
  /*
    Msg_t   Msg;

    Msg.expire = 0;
    Msg.prio = MSG_PRIO_HIGHEST;
    Msg.type = MSG_TYPE_TEST_CTRL;
    Msg.MsgCB = NULL;
    Msg.Data.TestCtrlDat.Cmd = cmd;
    Msg.Data.TestCtrlDat.Cmd_Act = cmd_act;
    Msg.Data.TestCtrlDat.Cmd_Para = cmd_para;
    SweepRobot_SendMsg(&Msg);
  */
}


/*********************************************END OF FILE**********************/
