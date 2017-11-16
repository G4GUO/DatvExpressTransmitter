#pragma once

void report_error(int code);
int get_error(void);

#define ERROR_INVALID_VIDEO_CAPTURE -1001
#define ERROR_INVALID_AUDIO_CAPTURE -1002
#define ERROR_FIRMWARE_NOT_FOUND    -1003
#define ERROR_FPGA_NOT_FOUND        -1004
#define ERROR_SDR_NOT_FOUND         -1005
#define ERROR_CONFIG_NOT_FOUND      -1006
#define ERROR_ILLEGAL_S2_VALUES     -1007


