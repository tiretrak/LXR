/*
 * VelocityModulation.c
 *
 *  Created on: 06.01.2013
 *      Author: Julian
 */


#include "modulationNode.h"
#include "DrumVoice.h"
#include "CymbalVoice.h"
#include "HiHat.h"
#include "Snare.h"
#include "sequencer.h"

 ModulationNode velocityModulators[6];


void modNode_init(ModulationNode* vm)
{
	vm->lastVal = 0;
	vm->amount = 0.f;
	modNode_setDestination(vm,0);
}

void modNode_setOriginalValueChanged(ModulationNode* vm, uint16_t idx)
{
	if(vm->destination == idx)
		{
			//parameter is active in this modulator
			//update orig value
			switch(parameterArray[idx].type)
			{
				case TYPE_UINT8:
					vm->originalValue.itg = *((uint8_t*)parameterArray[vm->destination].ptr);
					break;

				case TYPE_SPECIAL_F:
					break;

				case TYPE_SPECIAL_FILTER_F:
				case TYPE_FLT:
					vm->originalValue.flt = *((float*)parameterArray[vm->destination].ptr);
					break;

				case TYPE_SPECIAL_P:

					break;


				case TYPE_UINT32:
					vm->originalValue.itg = *((uint32_t*)parameterArray[vm->destination].ptr);
					break;

				default:
					break;
			}
			//calc new output value
			//modNode_updateValue(vm,vm->lastVal);

		}
}

void modNode_originalValueChanged(uint16_t idx)
{
	uint8_t i;
	for(i=0;i<6;i++)
	{
		modNode_setOriginalValueChanged(&velocityModulators[i],idx);
	}

	modNode_setOriginalValueChanged(&voiceArray[0].lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&voiceArray[1].lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&voiceArray[2].lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&snareVoice.lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&cymbalVoice.lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&hatVoice.lfo.modTarget,idx);




}
/*
void modNode_originalValueModulated(uint16_t idx, ModulationNode* modSource)
{
	uint8_t i;
	for(i=0;i<6;i++)
	{
		if(&velocityModulators[i] != modSource) {
			modNode_setOriginalValueChanged(&velocityModulators[i],idx);
		}
	}

	if(&voiceArray[0].lfo.modTarget != modSource) {
		modNode_setOriginalValueChanged(&voiceArray[0].lfo.modTarget,idx);
	}
	if(&voiceArray[1].lfo.modTarget != modSource) {
		modNode_setOriginalValueChanged(&voiceArray[1].lfo.modTarget,idx);
	}
	if(&voiceArray[2].lfo.modTarget != modSource) {
		modNode_setOriginalValueChanged(&voiceArray[2].lfo.modTarget,idx);
	}
	if(&snareVoice.lfo.modTarget != modSource) {
		modNode_setOriginalValueChanged(&snareVoice.lfo.modTarget,idx);
	}
	if(&cymbalVoice.lfo.modTarget != modSource) {
		modNode_setOriginalValueChanged(&cymbalVoice.lfo.modTarget,idx);
	}
	if(&hatVoice.lfo.modTarget != modSource) {
		modNode_setOriginalValueChanged(&hatVoice.lfo.modTarget,idx);
	}
}
*/

void modNode_resetTargets()
{
	uint8_t i;
	for(i=0;i<6;i++)
	{
		paramArray_setParameter(velocityModulators[i].destination,velocityModulators[i].originalValue);
	}

	paramArray_setParameter(voiceArray[0].lfo.modTarget.destination,voiceArray[0].lfo.modTarget.originalValue);
	paramArray_setParameter(voiceArray[1].lfo.modTarget.destination,voiceArray[1].lfo.modTarget.originalValue);
	paramArray_setParameter(voiceArray[2].lfo.modTarget.destination,voiceArray[2].lfo.modTarget.originalValue);
	paramArray_setParameter(snareVoice.lfo.modTarget.destination,snareVoice.lfo.modTarget.originalValue);
	paramArray_setParameter(cymbalVoice.lfo.modTarget.destination,cymbalVoice.lfo.modTarget.originalValue);
	paramArray_setParameter(hatVoice.lfo.modTarget.destination,hatVoice.lfo.modTarget.originalValue);


}

void modNode_reassignVeloMod()
{
	uint8_t i;
	for(i=0;i<6;i++)
	{
		modNode_updateValue(&velocityModulators[i], velocityModulators[i].lastVal);
	}
}


void modNode_setDestination(ModulationNode* vm, uint16_t dest)
{
	//TODO check if this interrupts other modulations too much
	//is needed to really get the original value
	modNode_resetTargets();

	//restore old value to original
	paramArray_setParameter(vm->destination,vm->originalValue);
	//update dest param
	vm->destination = dest;
	//get new original parameter value
	switch(parameterArray[vm->destination].type)
	{
		case TYPE_UINT8:
			vm->originalValue.itg = *((uint8_t*)parameterArray[vm->destination].ptr);
			break;
		case TYPE_SPECIAL_F:
			vm->originalValue.flt = 1;//*((float*)parameterArray[vm->destination].ptr);
			break;

		case TYPE_SPECIAL_FILTER_F:
		case TYPE_FLT:
			vm->originalValue.flt = *((float*)parameterArray[vm->destination].ptr);
			break;

		case TYPE_UINT32:
			vm->originalValue.itg = *((uint32_t*)parameterArray[vm->destination].ptr);
			break;

		case TYPE_SPECIAL_P:

			break;

		default:
			break;
	}


}

void modNode_updateValue(ModulationNode* vm, float val)
{

	vm->lastVal = val;
	switch(parameterArray[vm->destination].type)
	{
	case TYPE_UINT8:
//		(*((uint8_t*)parameterArray[vm->destination].ptr)) = vm->originalValue.itg * vm->amount * val + (1.f-vm->amount) * vm->originalValue.itg;
		(*((uint8_t*)parameterArray[vm->destination].ptr)) = (*((uint8_t*)parameterArray[vm->destination].ptr)) * vm->amount * val + (1.f-vm->amount) * (*((uint8_t*)parameterArray[vm->destination].ptr));
		break;

	case TYPE_UINT32:
			//*((uint32_t*)parameterArray[vm->destination].ptr) = vm->originalValue.itg * vm->amount * val + (1.f-vm->amount) * vm->originalValue.itg;
			(*((uint32_t*)parameterArray[vm->destination].ptr)) = (*((uint32_t*)parameterArray[vm->destination].ptr)) * vm->amount * val + (1.f-vm->amount) * (*((uint32_t*)parameterArray[vm->destination].ptr));
			break;

	case TYPE_SPECIAL_FILTER_F:
		{
			/*
			ResonantFilter *filter=0;
			float *out = (((float*)parameterArray[vm->destination].ptr));
			switch(vm->destination)
			{
			case PAR_FILTER_FREQ1:
				filter = &snareVoice.filter;
				break;
			case PAR_FILTER_FREQ2:
				filter = &cymbalVoice.filter;
				break;
			case PAR_FILTER_FREQ3:
				filter = &hatVoice.filter;
				break;
			case PAR_FILTER_FREQ_DRUM1:
				filter = &voiceArray[0].filter;
				break;
			case PAR_FILTER_FREQ_DRUM2:
				filter = &voiceArray[0].filter;
				break;
			case PAR_FILTER_FREQ_DRUM3:
				filter = &voiceArray[0].filter;
				break;
			}

			float originalFreq = filter->f;
			float newFreq = originalFreq * vm->amount * val + (1.f-vm->amount) * originalFreq;
			*out = (*out) * vm->amount * val + (1.f-vm->amount) * (*out);
			*/
		}
		break;

	case TYPE_FLT:
		//*((float*)parameterArray[vm->destination].ptr) = vm->originalValue.flt * vm->amount * val + (1.f-vm->amount) * vm->originalValue.flt;
		(*((float*)parameterArray[vm->destination].ptr)) = (*((float*)parameterArray[vm->destination].ptr)) * vm->amount * val + (1.f-vm->amount) * (*((float*)parameterArray[vm->destination].ptr));
		break;

	case TYPE_SPECIAL_F:
		//*((float*)parameterArray[vm->destination].ptr) = vm->originalValue.flt * vm->amount * val + (1.f-vm->amount) * vm->originalValue.flt;
		(*((float*)parameterArray[vm->destination].ptr)) = (*((float*)parameterArray[vm->destination].ptr)) * vm->amount * val + (1.f-vm->amount) * (*((float*)parameterArray[vm->destination].ptr));
		break;

	case TYPE_SPECIAL_P:

		break;

	default:
		break;

	}

	//modNode_originalValueModulated(vm->destination, vm);

}