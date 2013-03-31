/**
*****************************************************************************
**
**  File        : main.c
**
**  Abstract    : main function.
**
**  Functions   : main
**
**  Distribution: The file is distributed �as is,� without any warranty
**                of any kind.
**
**
**
*****************************************************************************
*/
#include "stm32f4xx.h"
#include "core_cm4.h"
#include <stdio.h>
#include <stdint.h>

#include "globals.h"
#include "config.h"
#include "wavetable.h"
#include "Oscillator.h"
#include "ResonantFilter.h"
#include "random.h"
#include "AudioCodecManager.h"
#include "sequencer.h"

#include "mixer.h"
#include "Uart.h"
#include "FIFO.h"
#include "DrumVoice.h"
#include "CymbalVoice.h"
#include "HiHat.h"
#include "Snare.h"
#include "EuklidGenerator.h"
#include "ParameterArray.h"
#include "modulationNode.h"

#if USE_SD_CARD
#include "SD_Manager.h"
#endif

#include "usb_manager.h"
#include "MidiParser.h"

//----------------------------------------------------------------
//stub function for newlib
void _exit(int status)
{
	_exit(status);
};

int _sbrk(int nbytes)
{
	return 0;
};

#define __MYPID 1
int
_getpid()
{
  return __MYPID;
}

/*
 * kill -- go out via exit...
 */
int _kill(int pid, int sig)
{
  if(pid == __MYPID)
    _exit(sig);
  return 0;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
// var to hold the clock infos
RCC_ClocksTypeDef RCC_Clocks;


float filFreq = .9f;
float sign = 1.f;
float tempo = 0.00005f;
float gain = 1.0f;
uint8_t fType = 0;
//----------------------------------------------------------------


void initAudioJackDiscoverPins()
{
	//l1 = pa0
	//r2 = PA5
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_5;

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);



	//r1 = pc4
	//l2 = pc5
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

}


//uint8_t calc = 0;
inline void calcNextSampleBlock()
{
#if USE_DAC2
	mixer_calcNextSampleBlock(&dma_buffer[bCurrentSampleValid*32],&dma_buffer2[bCurrentSampleValid*32]);
#else
	mixer_calcNextSampleBlock(&dma_buffer[bCurrentSampleValid*32],&dma_buffer2[(1-bCurrentSampleValid)*32]);
#endif
	bCurrentSampleValid = SAMPLE_VALID;
}


int main(void)
{
	/* get system clock info*/
	RCC_GetClocksFreq(&RCC_Clocks);
	/* set timebase systick to 1ms*/
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

	initAudioJackDiscoverPins();

	mixer_init();

	//precalc the first dma buffer block
	calcNextSampleBlock();

	// start the audio codec
	CodecInit(SYNTH_FS);

	parameterArray_init();

	int i;
	for(i=0;i<6;i++)
	{
		modNode_init(&velocityModulators[i]);
	}

	initMidiUart();

	initFrontpanelUart();

	initRng();

	euklid_init();

	//init the seq speed
	seq_init();
	seq_setBpm(120);

	initDrumVoice();
	Snare_init();
	HiHat_init();
	Cymbal_init();

#if USE_SD_CARD
	sdManager_init();
#endif

	usb_init();

	//--------------------------------------------------------------------
	//------------------------- Main Loop --------------------------------
	//--------------------------------------------------------------------
    while (1)
    {

    	usb_tick();
#if USE_SD_CARD
    	 //check if something needs to be send to the front
    	sdManager_tick();
#endif
    	//generate next sample if no valid sample is present
    	if(bCurrentSampleValid!= SAMPLE_VALID)
    	{
    		calcNextSampleBlock();
    	}

		//process midi
		uart_processMidi();

		//process incoming frontpanel data
		uart_processFront();

		//check if we have some usb midi messages and process them
		MidiMsg msg;
		if(usb_getMidi(&msg))
		{
			if( (msg.status&0xf0) == 0xf0)
			{
				//system message
				midiParser_handleSystemByte(msg.status);
			}
			else
			{
				midiParser_parseMidiMessage(msg);
			}
		}

		//process the sequencer
		seq_tick();

    }
}
