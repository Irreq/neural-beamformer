#ifndef _HMI_H_
#define _HMI_H_

#ifdef __cplusplus
extern "C" {
#endif

int start_hmi();
int stop_hmi();

void transmit(float *mimo, int n);
#ifdef __cplusplus
}
#endif

#endif