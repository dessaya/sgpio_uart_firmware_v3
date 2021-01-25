/**********************************************************************
 * $Id$		sgpio_uart_demo.c			2012-10-22
 *//**
 * @file		sgpio_uart_demo.c
 * @brief	This example describes how to using UART via SGPIO
 * @version	1.0
 * @date		22. Oct. 2012
 * @author	NXP MCU Application Team
 *
 * Copyright(C) 2012, NXP Semiconductor
 * All rights reserved.
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors'
 * relevant copyright in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 **********************************************************************/
#include <string.h>
#include <stdio.h>
#include "lpc43xx_cgu.h"
#include "lpc43xx_scu.h"
#include "sgpio_uart.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup SGPIO_Uart_demo	SGPIO_Uart_demo
 * @ingroup SGPIO Examples
 * @{
 */
mySGPIO_Type	mySGPIO;
/************************** PRIVATE DEFINITIONS *************************/
#define MAX_FULL_RX_NUM	1
#define MAX_TR_NUM			1

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] = "\n\r Hello! NXP Semiconductors";
#ifdef INT_RAM
uint8_t menu2[] = "\n\r SGPIO UART demo in internal RAM\n\r(MCU lpc43xx - ARM Cortex-M4) ";
#elif INT_FLASH
uint8_t menu2[] = "\n\r SGPIO UART demo in internal Flash\n\r(MCU lpc43xx - ARM Cortex-M4) ";
#else
uint8_t menu2[] = "\n\r SGPIO UART demo in undefined memory\n\r(MCU lpc43xx - ARM Cortex-M4) ";
#endif
uint8_t menu3[] = "\n\r<Press key>:\n\r [1]-> half duplex mode \n\r [2]-> full duplex mode\n\r";

uint8_t menu5[] = "Waiting for Rx... Press any key \n\r";
uint8_t menu6[] = "Sending... Press any key to receive data while sending \n\r";
/************************** PRIVATE FUNCTIONS *************************/

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief		Print Welcome menu
 * @param[in]	SGPIO slice for Tx pin
 * @return 		None
 **********************************************************************/
void print_menu(uint32_t SGPIO_TxSlice)
{
    SGPIO_UART_Send(SGPIO_TxSlice, menu1, sizeof(menu1));
    SGPIO_UART_Send(SGPIO_TxSlice, menu2, sizeof(menu2));
    SGPIO_UART_Send(SGPIO_TxSlice, menu3, sizeof(menu3));
}
/*********************************************************************//**
 * @brief		Configurate SGPIO pin and slice on one board
 * @param[in]	void 
 * @return 		None
 **********************************************************************/
void SGPIO_Borad_config(void)
{//for MCB4300 board using UART0
    scu_pinmux(0x2 ,0 , MD_BUK, FUNC0); 	// P2.0 : UART0 Tx with SGPIO4(slice C) dout
    scu_pinmux(0x2 ,1 , MD_PLN|MD_EZI|MD_ZI, FUNC0); 	// P2.1 : UART0 Rx with SGPIO5(slice K) din

    mySGPIO.TxPin	= SGPIO_4;
    mySGPIO.TxSlice = C;
    mySGPIO.RxPin	= SGPIO_5;
    mySGPIO.RxSlice = K;
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief		Main SGPIO UART program body
 * @param[in]	None
 * @return 		int
 **********************************************************************/
int c_entry(void)
{
    // UART Configuration structure variable
    SGPIO_UART_CFG_Type UARTConfigStruct;
    Status status;
    uint32_t idx, len, txlen, tmp;
    __IO FlagStatus exitflag;
    uint8_t buffer[50], buf[50], i;
    static uint8_t key;

    SystemInit();
    CGU_Init();

    /*
     * Configurate SGPIO pin/slice connect
     */
    SGPIO_Borad_config();

    /* Initialize UART Configuration parameter structure to default state:
     * Baudrate = 9600bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */
    SGPIO_UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize SGPIO UART Tx/Rx
    status = SGPIO_UART_setclk(mySGPIO.TxSlice, mySGPIO.RxSlice, &UARTConfigStruct);
    if(status == ERROR) return (-1);
    SGPIO_UART_Tx_Init(mySGPIO.TxPin, mySGPIO.TxSlice);
    SGPIO_UART_Rx_Init(mySGPIO.RxPin, mySGPIO.RxSlice);

    // print welcome screen
    print_menu(mySGPIO.TxSlice);

    //Enable SGPIO UART Rx
    SGPIO_UART_Setmode(mySGPIO.RxSlice, ENABLE);

    while(1)
    {
        //* choose test mode */
        exitflag = RESET;
        len = 0;
        key = 0;
        while (len == 0)
        {
            len = SGPIO_UART_Receive(mySGPIO.RxSlice, buffer, 1);
            key = buffer[0];
        }
        if(key == '1') 
        {
            SGPIO_UART_Send(mySGPIO.TxSlice, menu5, sizeof(menu5));
        }
        else if(key == '2') 
        {
            idx = 0;
            txlen = 0;
            len = 0;
            for(i=0;i<50;i++)
            {
                buffer[i] = 0;
                buf[i] = 0;
            }
            strcpy((char *)buf, " '");
            SGPIO_UART_Send(mySGPIO.TxSlice, menu6, sizeof(menu6));
        }
        else 
        {
            continue;
        }

        while (exitflag == RESET)
        {
            if(key == '1')//half duplex	
            {
                len = SGPIO_UART_Receive(mySGPIO.RxSlice, buffer, MAX_TR_NUM);
                /* Got some data */
                idx = 0;
                while (idx < len)
                {
                    if (buffer[idx] == 27)
                    {
                        /* ESC key, set exit flag */
                        SGPIO_UART_Send(mySGPIO.TxSlice, menu3, sizeof(menu3));
                        exitflag = SET;
                    }
                    else
                    {
                        /* Echo it back */
                        SGPIO_UART_Send(mySGPIO.TxSlice, &buffer[idx], MAX_TR_NUM);
                    }
                    idx += MAX_TR_NUM;
                }
            }
            else//full duplex
            {
                txlen = SGPIO_UART_SendFull(mySGPIO.TxSlice, menu6, sizeof(menu6), txlen);
                len = SGPIO_UART_RxFull(mySGPIO.RxSlice, buffer, idx);
                if(idx < len)//received
                {
                    tmp = 2;
                    strcpy((char *)&buf[tmp], (const char *)&buffer[idx++]);
                    strcpy((char *)&buf[++tmp], "' ");
                    SGPIO_UART_Send(mySGPIO.TxSlice, buf, tmp+2);//send the received data
                    SGPIO_UART_Setmode(mySGPIO.TxSlice, ENABLE);//enable Tx for continuous sending in SGPIO_UART_SendFull()
                }	
                else 
                {
                    if(txlen == sizeof(menu6))//finish sending one time
                    { 
                        txlen = 0;//for next sending
                        if(idx>(MAX_FULL_RX_NUM-1))//exit if received the required number of data
                        {
                            SGPIO_UART_Send(mySGPIO.TxSlice, menu3, sizeof(menu3));
                            idx = 0;
                            len = 0;
                            exitflag = SET;
                        }
                    }
                }
            }//full duplex
        }//while (exitflag == RESET)
    }//while(1);
}

/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
int main(void)
{
    return c_entry();
}


#ifdef  DEBUG
/*******************************************************************************
 * @brief		Reports the name of the source file and the source line number
 * 				where the CHECK_PARAM error has occurred.
 * @param[in]	file Pointer to the source file name
 * @param[in]    line assert_param error line source number
 * @return		None
 *******************************************************************************/
void check_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while(1);
}
#endif

/**
 * @}
 */
