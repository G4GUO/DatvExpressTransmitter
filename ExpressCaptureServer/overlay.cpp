#include "stdafx.h"
#include <stdint.h>
#include "Codecs.h"
#include "Dvb.h"
/*
void convert_rgb_yuv420p( uint8_t *rgb, int x, int y, int height, int width, AVFrame *frame )
{
    int rn,yn;
    rn = yn = 0;
    uint8_t r,g,b;

    // The Y plane
	for( int in_line = 0; in_line < x+height; line++ )
    {
		rn = line*frame->width*3;//RGB pointer (stride)
		for( int p = 0; p < width; p++ )
        {
            r  = 255;//rgb[rn+(p*3)];
            g  = 255;rgb[rn+(p*3)+1];
            b  = 255;rgb[rn+(p*3)+2];
            yn = line*frame->width + p;//YUV pointer
			frame->buf[0]->data[yn] = (uint8_t)((0.257 * r) + (0.504 * g) + (0.098 * b) + 16);//Y
        }
    }

    // U and V planes Half resolution in both planes
	for( int line = x; line < (x+height)/2; line++ )
    {
        rn = line*2*frame->width*3;// RGB pointer
		for( int p = 0; p < width/2; p++)
        {
            //Chrominance
            r  = 255;rgb[rn+(p*6)];
            g  = 255;rgb[rn+(p*6)+1];
            b  = 255;rgb[rn+(p*6)+2];
            yn = line*frame->width/2 + p;//YUV pointer
            frame->buf[1]->data[yn] = (uint8_t)(-(0.148 * r) - (0.291 * g) + (0.439 * b) + 128);//U
            frame->buf[2]->data[yn] = (uint8_t)( (0.439 * r) - (0.368 * g) - (0.071 * b) + 128);//V
        }
    }
}
*/
void convert_rgb_yuv420p( uint8_t *rgb, int x, int y, int height, int width, AVFrame *frame )
{
    int rn,yn;
    rn = yn = 0;
    uint8_t r,g,b;

	for(int i = 0; i < height; i++ ){
		int ip = (height-i-1)*width*3;
		int op = (((i+y)*frame->linesize[0]) + x);
		for( int p = 0; p < width; p++ ){
			b = rgb[ip+(p*3)];
			g = rgb[ip+(p*3)+1];
			r = rgb[ip+(p*3)+2];
			frame->buf[0]->data[op+p] = (uint8_t)((0.257 * r) + (0.504 * g) + (0.098 * b) + 16);//Y
		}
	}

	// U and V planes Half resolution in both planes
	for( int i = 0; i < height/2; i++ )
    {
		int ip = (height-(i*2)-1)*width*3;
		int op = (i+y/2)*frame->linesize[1] + x/2;
		for( int p = 0; p < width/2; p++)
        {
            //Chrominance
            b  = (rgb[ip+(p*6)]+rgb[ip+(p*6)+3])/2;
            g  = (rgb[ip+(p*6)+1]+rgb[ip+(p*6)+4])/2;
            r  = (rgb[ip+(p*6)+2]+rgb[ip+(p*6)+5])/2;
            frame->buf[1]->data[op+p] = (uint8_t)(-(0.148 * r) - (0.291 * g) + (0.439 * b) + 128);//U
            frame->buf[2]->data[op+p] = (uint8_t)( (0.439 * r) - (0.368 * g) - (0.071 * b) + 128);//V
        }
    }

}
int m_width;
int m_height;
BYTE *m_pbase = NULL;
HGDIOBJ m_hObj;

void overlay_frame( AVFrame *frame ){
	convert_rgb_yuv420p( m_pbase, 50, 50, m_height, m_width, frame );
}

void overlay_deinit(void){
	DeleteObject(m_hObj);
}

int overlay_init(const char *text){
	//choose the font
	CFont m_Font;
	LOGFONT* m_pLF;
	m_pLF=(LOGFONT*)calloc(1,sizeof(LOGFONT));
	strncpy_s(m_pLF->lfFaceName,"Courier New",31);
	m_pLF->lfHeight=40;
	m_pLF->lfWidth =0;
	m_pLF->lfWeight=100;
	m_pLF->lfItalic=0;
	m_pLF->lfUnderline=0;
	m_Font.CreateFontIndirect(m_pLF);

	m_width  = strlen(text)*32;
	m_height = 40;

	//create dc.
	HDC pDC = GetDC(NULL);
	HDC TmpDC=CreateCompatibleDC(pDC);

	BITMAPINFO bmInfo;
	memset(&bmInfo.bmiHeader,0,sizeof(BITMAPINFOHEADER));
	bmInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biWidth=m_width;
	bmInfo.bmiHeader.biHeight=m_height;
	bmInfo.bmiHeader.biPlanes=1;
	bmInfo.bmiHeader.biBitCount=24;

	//create a new bitmap and select it in the memory dc
	HBITMAP hBmp=CreateDIBSection(pDC,&bmInfo,DIB_RGB_COLORS,(void**)&m_pbase,0,0);
	m_hObj=SelectObject(TmpDC,hBmp);

	CDC dc;
	dc.Attach(TmpDC);


	//select the font in the dc

	CFont* pOldFont=NULL;
	if (m_Font.m_hObject) 
		dc.SelectObject(&m_Font);
	else 
		dc.SelectObject(GetStockObject(DEFAULT_GUI_FONT));

	dc.SetBkColor(RGB(0,0,0));
	//dc.SelectObject(GetStockObject(BLACK_BRUSH));
	//dc.SelectObject(GetStockObject(WHITE_PEN));

	//dc.Rectangle( 0, 0, 200,200);
	dc.SetTextColor(RGB(255,255,0));
	int width = dc.GetOutputTextExtent(text,strlen(text)).cx;
	int height = dc.GetOutputTextExtent(text,strlen(text)).cy;
	RECT pos = { 0, 0, m_width, m_height};
	dc.DrawText(text,-1,&pos,DT_CENTER);
	free(m_pLF);
	dc.Detach();
	return 0;
}