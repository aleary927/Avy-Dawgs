#include "main.h"
#include "dsp.h"
#include <arm_math.h>

// https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
float goertzel_power(int16_t *data)
{
  const float scaling_factor = ((float) BUF_SIZE) / 2.0;
  const float target_freq = 457000.0;
  const float sampling_rate = 1800000.0;
  const int32_t k = (int32_t) (0.5 + ((((float) BUF_SIZE) * target_freq) / sampling_rate));
  const float omega = (2.0 * 3.1415926 * k) / ((float) BUF_SIZE);

  const float sine = arm_cos_f32(omega); 
  const float cosine = arm_cos_f32(omega);
  const float coeff = 2 * cosine;

  float q0 = 0;
  float q1 = 0; 
  float q2 = 0; 

  for (int i = 0; i < BUF_SIZE; i++) {
    q0 = ((float) data[i]) + coeff * q1 - q2; 
    
    // rotate data
    q2 = q1; 
    q1 = q0;
  }

  float real = (q1 * cosine - q2) / scaling_factor; 
  float imag = (q1 * sine) / scaling_factor;

  return real*real + imag*imag;
}
