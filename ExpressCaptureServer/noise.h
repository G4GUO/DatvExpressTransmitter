#pragma once
#include "Dvb.h"
#include "express.h"
#define FLT double

FLT noise_get_sn(void);
int noise_is_enabled(void);
void noise_on(void);
void noise_off(void);
void noise_init(void);
void noise_set_sn(FLT sn);
scmplx *noise_add(scmplx *s, int len);
