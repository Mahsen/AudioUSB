//************************************************************* means *******************************************************************//

//************************************************************ includes *****************************************************************//
#include "Process.h"
//************************************************************ variables ****************************************************************//
uint8_t  Mute;                                 /* Mute State */
uint32_t Volume;                               /* Volume Level */

#if USB_DMA
uint32_t *InfoBuf = (uint32_t *)(DMA_BUF_ADR);
short *DataBuf = (short *)(DMA_BUF_ADR + 4*P_C);
#else
uint32_t InfoBuf[P_C];
short DataBuf[B_S];                             /* Data Buffer */
#endif

uint16_t  DataOut;                              /* Data Out Index */
uint16_t  DataIn;                               /* Data In Index */

uint8_t   DataRun;                              /* Data Stream Run State */
uint16_t  PotVal;                               /* Potenciometer Value */
uint32_t  VUM;                                  /* VU Meter */
uint32_t  Tick;                                 /* Time Tick */
//************************************************************ functions ****************************************************************//

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

  LPC_TIM0->IR = 1;                         /* Clear Interrupt Flag */
}

int Run_Processes()
{
    volatile uint32_t pclkdiv, pclk;
    SystemInit();
    
    UART_Init(2,115200,8,1,Parity_none);
    UART_PrintString(2,"Start . . . \r\n");
    

    UART_PrintString(2,"Configed\r\n"); 


    LPC_PINCON->PINSEL1 &=~((0x03<<18)|(0x03<<20));  
    /* P0.25, A0.0, function 01, P0.26 AOUT, function 10 */
    LPC_PINCON->PINSEL1 |= ((0x01<<18)|(0x02<<20));
    
    LPC_GPIO2->FIODIR   |= 1 << 0;                   /* P2.0 defined as Output (LED) */
    LPC_PINCON->PINSEL3 |=  (3ul<<30);               /* P1.31 is AD0.5 */
    LPC_SC->PCONP       |=  (1<<12);                 /* Enable power to ADC block */
    LPC_ADC->ADCR        =  (1<< 5) |                /* select AD0.5 pin */
                          (4<< 8) |                  /* ADC clock is 25MHz/5 */
                          (1<<21);                   /* enable ADC */
    
    /* Enable CLOCK into ADC controller */
    //  LPC_SC->PCONP |= (1 << 12);
    
    //  LPC_ADC->ADCR = 0x00200E04;		/* ADC: 10-bit AIN2 @ 4MHz */
    LPC_DAC->DACR = 0x00008000;		/* DAC Output set to Middle Point */
    
    /* By default, the PCLKSELx value is zero, thus, the PCLK for
    all the peripherals is 1/4 of the SystemFrequency. */
    /* Bit 2~3 is for TIMER0 */
    pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;
    switch ( pclkdiv )
    {
        case 0x00:
        default:
          pclk = SystemCoreClock/4;
        break;
        case 0x01:
          pclk = SystemCoreClock;
        break; 
        case 0x02:
          pclk = SystemCoreClock/2;
        break; 
        case 0x03:
          pclk = SystemCoreClock/8;
        break;
    }
    
    LPC_TIM0->MR0 = pclk/DATA_FREQ - 1;	/* TC0 Match Value 0 */
    LPC_TIM0->MCR = 3;					/* TCO Interrupt and Reset on MR0 */
    LPC_TIM0->TCR = 1;					/* TC0 Enable */
    NVIC_EnableIRQ(TIMER0_IRQn);
    
    
    USB_Init();			/* USB Initialization */
    USB_Connect(TRUE);		/* USB Connect */
    
    /********* The main Function is an endless loop ***********/ 
    while( 1 ); 
  
    
}



//_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_