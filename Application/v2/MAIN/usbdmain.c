/*----------------------------------------------------------------------------
 * Name:    usbmain.c
 * Purpose: USB Audio Class Demo
 * Version: V1.20
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC microcontroller devices only. Nothing else 
 *      gives you the right to use this software.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "../common/inc/LPC17xx.h"                        /* LPC17xx definitions */
#include "../common/inc/type.h"

/* Includes ------------------------------------------------------------------*/
//#include "../Sound/WavSample.c"
#include "../LPC1700CMSIS_Firmware_Library/include/lpc17xx_timer.h"
#include "../LPC1700CMSIS_Firmware_Library/include/lpc17xx_libcfg_default.h"
#include "../LPC1700CMSIS_Firmware_Library/include/lpc17xx_gpio.h"
#include "../LPC1700CMSIS_Firmware_Library/include/lpc17xx_pinsel.h"
#include "../LPC1700CMSIS_Firmware_Library/include/lpc17xx_dac.h"

#include "../Drivers/UART/uart.h"

//#include "../Drivers/GPU/GLCD/GLCD.h" 
//#include "../Drivers/GPU/GLCD/images.h" 

#include "../Drivers/USB/usb.h"
#include "../Drivers/USB/usbcfg.h"
#include "../Drivers/USB/usbhw.h"
#include "../Drivers/USB/usbcore.h"
#include "../Drivers/USB/usbaudio.h"


//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
char mode = 0;
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
typedef union 
{
	uint16_t Val;
	struct
	{ 
	uint16_t low	 :8;				
	uint16_t high	 :8;			
	}bits;	    
}wav;
uint32_t index;
wav temp;	
void DAC_Configuration(void);
void TIM_Configuration(void);
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
uint8_t  Mute;                                 /* Mute State */
uint32_t Volume;                               /* Volume Level */

#if USB_DMA
uint32_t *InfoBuf = (uint32_t *)(DMA_BUF_ADR);
short *DataBuf = (short *)(DMA_BUF_ADR + 4*P_C);
#else
uint32_t InfoBuf[P_C];
short DataBuf[B_S];                         /* Data Buffer */
#endif

uint16_t  DataOut;                              /* Data Out Index */
uint16_t  DataIn;                               /* Data In Index */

uint8_t   DataRun;                              /* Data Stream Run State */
uint16_t  PotVal;                               /* Potenciometer Value */
uint32_t  VUM;                                  /* VU Meter */
uint32_t  Tick;                                 /* Time Tick */
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------



/*
 * Get Potenciometer Value
 */

void get_potval (void) {
  uint32_t val;

  LPC_ADC->ADCR |= 0x01000000;              /* Start A/D Conversion */
  do {
    val = LPC_ADC->ADGDR;                   /* Read A/D Data Register */
  } while ((val & 0x80000000) == 0);        /* Wait for end of A/D Conversion */
  LPC_ADC->ADCR &= ~0x01000000;             /* Stop A/D Conversion */


  PotVal = ((val >> 8) & 0xF8) +            /* Extract Potenciometer Value */
           ((val >> 7) & 0x08);

}


/*
 * Timer Counter 0 Interrupt Service Routine
 *   executed each 31.25us (32kHz frequency)
 */

void TIMER0_IRQHandler(void) 
{
	if(!mode)
	{

		//if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)== SET)
		//{
		//    temp.bits.high = 0;
		//    temp.bits.low = WAV_DATA[index++];
		//
		//	DAC_UpdateValue ( LPC_DAC,temp.Val<<2);
		//	if( index >= sizeof(WAV_DATA) )
		//	{
		//	    index = 0;
	   	  volatile uint32_t pclkdiv, pclk;
		  LPC_PINCON->PINSEL1 &=~((0x03<<18)|(0x03<<20));  
		  /* P0.25, A0.0, function 01, P0.26 AOUT, function 10 */
		  LPC_PINCON->PINSEL1 |= ((0x01<<18)|(0x02<<20));
		
		  //LPC_GPIO2->FIODIR   |= 1 << 0;                    /* P2.0 defined as Output (LED) */
		  LPC_PINCON->PINSEL3 |=  (3ul<<30);                   /* P1.31 is AD0.5 */
		  LPC_SC->PCONP       |=  (1<<12);                   /* Enable power to ADC block */
		  LPC_ADC->ADCR        =  (1<< 5) |                  /* select AD0.5 pin */
		                          (4<< 8) |                  /* ADC clock is 25MHz/5 */
		                          (1<<21);                   /* enable ADC */

		  LPC_DAC->DACR = 0x00008000;		/* DAC Output set to Middle Point */

		  /* Bit 2~3 is for TIMER0 */
		  pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;
		  switch ( pclkdiv )
		  {
			case 0x00:
			default:
			  pclk = SystemFrequency/4;
			break;
			case 0x01:
			  pclk = SystemFrequency;
			break; 
			case 0x02:
			  pclk = SystemFrequency/2;
			break; 
			case 0x03:
			  pclk = SystemFrequency/8;
			break;
		  }
		
		  LPC_TIM0->MR0 = pclk/DATA_FREQ - 1;	/* TC0 Match Value 0 */
		  LPC_TIM0->MCR = 3;					/* TCO Interrupt and Reset on MR0 */
		  LPC_TIM0->TCR = 1;					/* TC0 Enable */
		  NVIC_EnableIRQ(TIMER0_IRQn);

		USB_Init();				/* USB Initialization */
        USB_Connect(TRUE);		/* USB Connect */

				mode = 1;
		//	}
		//}  

	}
	else
	{
	
	  long  val;
	  uint32_t cnt;
	
	  if (DataRun) {                            /* Data Stream is running */
	    val = DataBuf[DataOut];                 /* Get Audio Sample */
	    cnt = (DataIn - DataOut) & (B_S - 1);   /* Buffer Data Count */
	    if (cnt == (B_S - P_C*P_S)) {           /* Too much Data in Buffer */
	      DataOut++;                            /* Skip one Sample */
	    }
	    if (cnt > (P_C*P_S)) {                  /* Still enough Data in Buffer */
	      DataOut++;                            /* Update Data Out Index */
	    }
	    DataOut &= B_S - 1;                     /* Adjust Buffer Out Index */
	    if (val < 0) VUM -= val;                /* Accumulate Neg Value */
	    else         VUM += val;                /* Accumulate Pos Value */
	    val  *= Volume;                         /* Apply Volume Level */
	    val >>= 16;                             /* Adjust Value */
	    val  += 0x8000;                         /* Add Bias */
	    val  &= 0xFFFF;                         /* Mask Value */
	  } else {
	    val = 0x8000;                           /* DAC Middle Point */
	  }
	
	  if (Mute) {
	    val = 0x8000;                           /* DAC Middle Point */
	  }
	
	  LPC_DAC->DACR = val & 0xFFC0;             /* Set Speaker Output DACR=[1111111111000000] */
	
	  if ((Tick++ & 0x03FF) == 0) {             /* On every 1024th Tick */
	    get_potval();                           /* Get Potenciometer Value */
	    if (VolCur == 0x8000) {                 /* Check for Minimum Level */
	      Volume = 0;                           /* No Sound */
	    } else {
	      Volume = VolCur * PotVal;             /* Chained Volume Level */
	    }
	    val = VUM >> 20;                        /* Scale Accumulated Value */
	    VUM = 0;                                /* Clear VUM */
	    if (val > 7) val = 7;                   /* Limit Value */
	  }
	
	
	}
	
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}



/*****************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{

  SystemInit(); 





  UART_Init(2,115200,8,1,Parity_none);
  UART_PrintString(2,"Start . . . \r\n");

  DAC_Configuration();
  TIM_Configuration();

  index = 0;

  //LCD_Initializtion();
  //LCD_Clear(Blue);
  //LCD_WritePic(0,0,pic1.H_Size,pic1.V_Size,20,pic1.Streem);


  /********* The main Function is an endless loop ***********/ 
  while( 1 ); 
}





void DAC_Configuration(void)
{
	PINSEL_CFG_Type PinCfg;

	/*
	 * Init DAC pin connect
	 * AOUT on P0.26
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	/* init DAC structure to default
	 * Maximum	current is 700 uA
	 * First value to AOUT is 0
	 */
	DAC_Init(LPC_DAC);
}

/*******************************************************************************
* Function Name  : TIM_Configuration
* Description    : TIM_Configuration program.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void TIM_Configuration(void)
{
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	TIM_MATCHCFG_Type TIM_MatchConfigStruct;

	/* Initialize timer 0, prescale count time of 100uS */
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 10;

	/* use channel 0, MR0 */
	TIM_MatchConfigStruct.MatchChannel = 0;
	/* Enable interrupt when MR0 matches the value in TC register */
	TIM_MatchConfigStruct.IntOnMatch   = TRUE;
	/* Enable reset on MR0: TIMER will reset if MR0 matches it */
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;
	/* Stop on MR0 if MR0 matches it */
	TIM_MatchConfigStruct.StopOnMatch  = FALSE;
	/* Toggle MR0.0 pin if MR0 matches it */
	TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_TOGGLE;
	/* Set Match value, count value of 4 (4 * 10uS = 40us = 1s --> 25 KHz) */
	TIM_MatchConfigStruct.MatchValue   = 4 ;

	/* Set configuration for Tim_config and Tim_MatchConfig */
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
	TIM_ConfigMatch(LPC_TIM0,&TIM_MatchConfigStruct);

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(TIMER0_IRQn, ((0x01<<3)|0x01));
	/* Enable interrupt for timer 0 */
	NVIC_EnableIRQ(TIMER0_IRQn);
	/* To start timer 0 */
	TIM_Cmd(LPC_TIM0,ENABLE);
}

/******************************************************************************
**                            End Of File
******************************************************************************/
