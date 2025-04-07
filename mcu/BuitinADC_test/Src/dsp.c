#include "main.h"
#include "dsp.h"

// https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
float32_t goertzel(int16_t *data)
{
  const float32_t scaling_factor = ((float32_t) BUF_SIZE) / 2.0;
  const float32_t target_freq = 457000.0;
  const float32_t sampling_rate = 4000000.0;
  const int32_t k = (int32_t) (0.5 + ((((float32_t) BUF_SIZE) * target_freq) / sampling_rate));
  const float32_t omega = (2.0 * 3.1415926 * k) / ((float32_t) BUF_SIZE);

  const float32_t sine = arm_cos_f32(omega); 
  const float32_t cosine = arm_cos_f32(omega);
  const float32_t coeff = 2 * cosine;

  float32_t q0 = 0;
  float32_t q1 = 0; 
  float32_t q2 = 0; 

  for (int i = 0; i < BUF_SIZE; i++) {
    q0 = ((float32_t) data[i]) + coeff * q1 - q2; 
    
    // rotate data
    q2 = q1; 
    q1 = q0;
  }

  float32_t real = (q1 * cosine - q2); 
  float32_t imag = (q1 * sine);

  float32_t retval; 
  arm_sqrt_f32((real*real + imag*imag), &retval);
  return retval;
}
