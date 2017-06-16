#include "stdafx.h"
#include <stdint.h>

static int g_logging;
static FILE *g_fp;

void tp_log( uint8_t *tp ){
	if(g_logging==1) fwrite(tp, 1, 188, g_fp);
}
void start_log(void){
	char directory[250];
	if(GetCurrentDirectory(250,directory)){
		strncat_s(directory,"\\datvexpress.ts",17);
		if(fopen_s(&g_fp,directory,"wb") == 0 )
			g_logging = 1;
		else
			g_logging = 0;
	}
}
void stop_log(void){
	if(g_logging) fclose(g_fp);
	g_logging = 0;
}