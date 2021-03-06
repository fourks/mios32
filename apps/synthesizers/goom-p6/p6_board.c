// $Id: mios32_board.c 2035 2014-08-18 20:16:47Z tk $
//! \defgroup P6_BOARD
//!
//! Development Board specific functions for Audiothingies P6
//!
//! \{
/* ==========================================================================
 *
 *  Copyright (C) 2017 Ricard Wanderlof (polluxsynth@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <mios32.h>


/////////////////////////////////////////////////////////////////////////////
// Local variables
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//! Internally used help function to initialize a pin
/////////////////////////////////////////////////////////////////////////////
static s32 MIOS32_BOARD_PinInitHlp(GPIO_TypeDef *port, u16 pin_mask, mios32_board_pin_mode_t mode)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = pin_mask;

  switch( mode ) {
  case MIOS32_BOARD_PIN_MODE_IGNORE:
    return 0; // don't touch
  case MIOS32_BOARD_PIN_MODE_ANALOG:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    break;
  case MIOS32_BOARD_PIN_MODE_INPUT:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    break;
  case MIOS32_BOARD_PIN_MODE_INPUT_PD:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    break;
  case MIOS32_BOARD_PIN_MODE_INPUT_PU:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    break;
  case MIOS32_BOARD_PIN_MODE_OUTPUT_PP:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    break;
  case MIOS32_BOARD_PIN_MODE_OUTPUT_OD:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    break;
  default:
    return -2; // invalid pin mode
  }

  // init IO mode
  GPIO_Init(port, &GPIO_InitStructure);

  return 0; // no error
}

/////////////////////////////////////////////////////////////////////////////
// On-Board LEDs
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//! Initializes LEDs of the board
//! \param[in] leds mask contains a flag for each LED which should be initialized<BR>
//! <UL>
//!   <LI>STM32F4DISCOVERY: 4 LEDs (flag 0: green, flag1: orange, flag2: red, flag3: blue)
//! </UL>
//! \return 0 if initialisation passed
//! \return -1 if no LEDs specified for board
//! \return -2 if one or more LEDs not available on board
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_LED_Init(u32 leds)
{
  if( leds & 1 ) {
    MIOS32_BOARD_PinInitHlp(GPIOC, GPIO_Pin_2, MIOS32_BOARD_PIN_MODE_OUTPUT_PP); // LED1
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
//! Sets one or more LEDs to the given value(s)
//! \param[in] leds mask contains a flag for each LED which should be changed
//! \param[in] value contains the value which should be set
//! \return 0 if initialisation passed
//! \return -1 if no LEDs specified for board
//! \return -2 if one or more LEDs not available on board
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_LED_Set(u32 leds, u32 value)
{
  if( leds & 1 ) { // LED1
    MIOS32_SYS_STM_PINSET(GPIOC, GPIO_Pin_2, value & 1);
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
//! Returns the status of all LEDs
//! \return status of all LEDs
/////////////////////////////////////////////////////////////////////////////
u32 MIOS32_BOARD_LED_Get(void)
{
  u32 values = 0;

  if( GPIOC->ODR & GPIO_Pin_2 ) // LED1
    values |= (1 << 0);

  return values;
}


#if 0
/////////////////////////////////////////////////////////////////////////////
//! This function enables or disables one of the two DAC channels provided by 
//! STM32F103RE (and not by STM32F103RB).
//!
//! <UL>
//!  <LI>the first channel (chn == 0) is output at pin RA4 (J16:RC1 of the MBHP_CORE_STM32 module)
//!  <LI>the second channel (chn == 1) is output at pin RA5 (J16:SC of the MBHP_CORE_STM32 module)
//! </UL>
//! 
//! \param[in] chn channel number (0 or 1)
//! \param[in] enable 0: channel disabled, 1: channel enabled.
//! \return < 0 if DAC channel not supported (e.g. STM32F103RB)
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_DAC_PinInit(u8 chn, u8 enable)
{
#if defined(MIOS32_PROCESSOR_STM32F103RB)
  return -1; // generally not supported. Try DAC access for all other processors
#else
  if( chn >= 2 )
    return -1; // channel not supported

  if( enable ) {
    // enable DAC clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    // Once the DAC channel is enabled, the corresponding GPIO pin is automatically 
    // connected to the DAC converter. In order to avoid parasitic consumption, 
    // the GPIO pin should be configured in analog
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

    // init DAC
    DAC_InitTypeDef            DAC_InitStructure;
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;

    switch( chn ) {
      case 0:
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Cmd(DAC_Channel_1, ENABLE);
	break;

      case 1:
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	DAC_Cmd(DAC_Channel_2, ENABLE);
	break;

      default:
	return -2; // unexpected (chn already checked above)
    }
    
  } else {
    // disable DAC channel
    switch( chn ) {
      case 0: DAC_Cmd(DAC_Channel_1, DISABLE); break;
      case 1: DAC_Cmd(DAC_Channel_2, DISABLE); break;
      default:
	return -2; // unexpected (chn already checked above)
    }
  }

  return 0; // no error
#endif
}


/////////////////////////////////////////////////////////////////////////////
//! This function sets an output channel to a given 16-bit value.
//!
//! Note that actually the DAC will work at 12-bit resolution. The lowest
//! 4 bits are ignored (reserved for future STM chips).
//! \param[in] chn channel number (0 or 1)
//! \param[in] value the 16-bit value (0..65535). Lowest 4 bits are ignored.
//! \return < 0 if DAC channel not supported (e.g. STM32F103RB)
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_DAC_PinSet(u8 chn, u16 value)
{
#if defined(MIOS32_PROCESSOR_STM32F103RB)
  return -1; // generally not supported. Try DAC access for all other processors
#else
  switch( chn ) {
    case 0:
      DAC_SetChannel1Data(DAC_Align_12b_L, value);
      DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);
      break;

    case 1:
      DAC_SetChannel2Data(DAC_Align_12b_L, value);
      DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);
      break;

    default:
      return -1; // channel not supported
  }

  return 0; // no error
#endif
}
#endif

//! \}
