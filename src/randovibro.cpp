/*
 * File: rando vibro.cpp
 *
 * Test SDRAM memory i/o for delay lines
 *
 * 
 * 
 * 2018 (c) Korg
 *
 */

#include "usermodfx.h"
#include "delayline.hpp"
#include "simplelfo.hpp"


static dsp::SimpleLFO s_lfo;
static dsp::DualDelayLine s_delay;
static __sdram f32pair_t s_delay_ram[8192];

static uint8_t s_lfo_wave;
static float s_param_z, s_param;

static float s_len_z, s_len;
static const float s_fs_recip = 1.f / 48000.f;
static float lfo_time;

void MODFX_INIT(uint32_t platform, uint32_t api)
{
  s_delay.setMemory(s_delay_ram, 8192); 
  s_len = s_len_z = 4800.f;

  s_lfo.reset();
  s_lfo.setF0(1.f,s_fs_recip);
}

void MODFX_PROCESS(const float *main_xn, float *main_yn,
                   const float *sub_xn,  float *sub_yn,
                   uint32_t frames)
{
  
  const float * mx = main_xn;
  float * __restrict my = main_yn;
  const float * my_e = my + 2*frames;

  const float *sx = sub_xn;
  float * __restrict sy = sub_yn;
  
  const float len = s_len;
  float len_z = s_len_z;

  const float p = s_param;
  float p_z = s_param_z;
 	
	if (_fx_rand() <= 500000)
	{
		s_lfo.setF0(0.01f + 10.f * lfo_time, s_fs_recip);
	}
	else
	{
		s_lfo.setF0(0.01f + 10.f * lfo_time * _fx_white(), s_fs_recip);
	} 

  
  for (; my != my_e; ) {
	  

    
	p_z = linintf(0.002f, p_z, p);    
	s_lfo.cycle();
	float wave = s_lfo.sine_bi();
    wave *= 0.025f * s_param;
	
	
	
    *(my) = *(mx);
    *(sy) = *(sx);

    len_z = linintf(0.00004f, len_z, len);
	len_z += wave;
    const f32pair_t r = s_delay.readFrac(len_z);
    s_delay.write((f32pair_t){*(mx++), *(sx++)});

    *(my++) = r.a;
    *(sy++) = r.b;
  }
  
  s_param_z = p_z;
  s_len_z = len_z;
}


void MODFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case k_user_modfx_param_time:
    lfo_time = valf;
    break;
  case k_user_modfx_param_depth:
    s_param = valf * 5.f;
	break;
  default:
    break;
  }
}

