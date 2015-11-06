#ifndef __USART1_H
#define __USART1_H

#include "stm32f4xx.h"
#include <stdio.h>

enum USARTTestCtrlCmd {
    
    TEST_CTRL_CMD_TEST                      = 0x00,
    
    TEST_CTRL_CMD_LWHEEL                    = 0x10, 
    TEST_CTRL_CMD_RWHEEL                    = 0x11,
    TEST_CTRL_CMD_LBRUSH                    = 0x12,
    TEST_CTRL_CMD_RBRUSH                    = 0x13,
    TEST_CTRL_CMD_MBRUSH                    = 0x14,
    TEST_CTRL_CMD_FAN                       = 0x15,
    
    TEST_CTRL_CMD_SENSOR                    = 0x20,
    /*
    TEST_CTRL_CMD_IFRD_LED                  = 0x21,
    TEST_CTRL_CMD_FRONT                     = 0x22,
    TEST_CTRL_CMD_BOTTOM                    = 0x23,
    TEST_CTRL_CMD_COLLISION                 = 0x24,
    TEST_CTRL_CMD_UNI_WHEEL                 = 0x25,
    TEST_CTRL_CMD_WHEEL_FLOAT               = 0x26,
    TEST_CTRL_CMD_ASH_TRAY_INS              = 0x27,
    TEST_CTRL_CMD_ASH_TRAY_LVL              = 0x28,
    */
    
    TEST_CTRL_CMD_RGB_LED                   = 0x30,
    TEST_CTRL_CMD_KEY                       = 0x31,
    TEST_CTRL_CMD_IRDA                      = 0x32,
    TEST_CTRL_CMD_BUZZER                    = 0x33,
    
    TEST_CTRL_CMD_CHARGE                    = 0x40,
    /*
    TEST_CTRL_CMD_BAT_VOL                   = 0x41,
    TEST_CTRL_CMD_BAT_CUR                   = 0x42,
    */
};

enum USARTTestCtrlCmdAct {
    
    TEST_CTRL_CMD_ACT_ON                    = 0x10,
    TEST_CTRL_CMD_ACT_OFF                   = 0x11,
    TEST_CTRL_CMD_ACT_READ                  = 0x12,
    TEST_CTRL_CMD_ACT_DIR                   = 0x13,
    TEST_CTRL_CMD_ACT_SPEED                 = 0x14,
    TEST_CTRL_CMD_ACT_IFRD_LED              = 0x15,
    TEST_CTRL_CMD_ACT_B_SWITCH              = 0x16,
    TEST_CTRL_CMD_ACT_START                 = 0x17,
    TEST_CTRL_CMD_ACT_STOP                  = 0x18,
};

typedef struct USARTTestCtrlData{

    enum USARTTestCtrlCmd       Cmd;
    enum USARTTestCtrlCmdAct    Cmd_Act;
    int                         Cmd_Para;
} USARTTestCtrlData_t;

#define USART_RX_STATE_REC_FINISH_POS            15
#define USART_RX_STATE_CR_REC_POS                14
#define USART_RX_STATE_DASH_REC_POS              13
#define USART_RX_STATE_CMD_ACT_REC_EN_POS        12
#define USART_RX_STATE_CMD_PARA_REC_EN_POS       11
#define USART_RX_STATE_CNT_MASK                  0x07FF
#define USART_RX_STATE_CNT_CLR_MASK              0xF800
#define IS_UART4_GET_DATA_FINISH()              (UsartRxState&(1<<USART_RX_STATE_REC_FINISH_POS)?1:0)

extern u16 UsartRxState;

void USART1_Config(void);
void USART2_Config(void);
void UART4_Config(void);
void USART_CmdProc(void);
//int fputc(int ch, FILE *f);
//void USART1_printf(USART_TypeDef* USARTx, uint8_t *Data,...);

#endif /* __USART1_H */
