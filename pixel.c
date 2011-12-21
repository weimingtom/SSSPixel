#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "raw.h"

#define TEST_PIXEL 1

//---------------------------------------------
#ifdef _MSC_VER

static HDC s_hBackBufferDC;
static unsigned char *s_pixel;
static int s_width;
static int s_height;

static void draw(HWND hAppWnd, int width, int height)
{
	int x, y;
	unsigned char *pixel = s_pixel;
	//draw
	Rectangle(s_hBackBufferDC, 0, 0, width, height);
	for (y = 0; y < s_height; y++)
	{
		for (x = 0; x < s_width; x++)
		{
			unsigned char b = *(pixel++);
			unsigned char g = *(pixel++);
			unsigned char r = *(pixel++);
			SetPixel(s_hBackBufferDC, x, s_height - y, RGB(r, g, b));
		}
	}
	InvalidateRect(hAppWnd, 0, FALSE);
}

static LRESULT CALLBACK WndProc(HWND hWnd, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam)
{
	static HBITMAP s_hBitmap;
	static HBITMAP s_hOldBitmap;
	static HPEN s_hPen;
	static HPEN s_hOldPen;
	static HBRUSH s_hBrush;
	static HBRUSH s_hOldBrush;
	static HDC s_hCanvas;

	switch(message)
	{
	case WM_COMMAND:
		break;

	case WM_CREATE:
		{
			RECT rect;
			HDC hDC;
			GetClientRect(hWnd, &rect);
			hDC = GetDC(hWnd);
			s_hBitmap = CreateCompatibleBitmap(hDC, 
				rect.right - rect.left, 
				rect.bottom - rect.left);
			s_hPen = CreatePen(PS_SOLID, 0, 0);
			s_hBackBufferDC = CreateCompatibleDC(hDC);
			s_hBrush = (HBRUSH)CreateSolidBrush(0);
			/*
			 * NOTE: bitmap object must be selected
			 */
			s_hOldBitmap = (HBITMAP)SelectObject(s_hBackBufferDC, s_hBitmap);
			s_hOldPen = (HPEN)SelectObject(s_hBackBufferDC, s_hPen);
			s_hOldBrush = (HBRUSH)SelectObject(s_hBackBufferDC, s_hBrush);
			ReleaseDC(hWnd, hDC);
			s_hCanvas = s_hBackBufferDC;
		}
		break;
	
	case WM_DESTROY:
		{
			SelectObject(s_hBackBufferDC, s_hOldPen);
			SelectObject(s_hBackBufferDC, s_hOldBitmap);
			SelectObject(s_hBackBufferDC, s_hOldBrush);
			DeleteObject(s_hBackBufferDC);
			DeleteObject(s_hBitmap);
			DeleteObject(s_hPen);
			DeleteObject(s_hBrush);
		}
		PostQuitMessage(0);
		break;
	
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			BitBlt(hdc,
				ps.rcPaint.left, ps.rcPaint.top,
				ps.rcPaint.right - ps.rcPaint.left, 
				ps.rcPaint.bottom - ps.rcPaint.top,
				s_hCanvas, 
				ps.rcPaint.left, ps.rcPaint.top, 
				SRCCOPY);
			EndPaint(hWnd, &ps);
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void testwin(unsigned char *pixel, int w/*pixel*/, int h/*pixel*/, 
			 int width/*window*/, int height/*window*/)
{
	const DWORD WINDOW_STYLE = WS_OVERLAPPED | WS_CAPTION | 
		WS_MINIMIZEBOX | WS_SYSMENU;
	const char *WINDOW_CLASS = "pixelwin";
	const char *WINDOW_TITLE = "pixelwin";
	HINSTANCE hInstance = GetModuleHandle(NULL);
	HWND hAppWnd = NULL;
    WNDCLASSEX wcex = { 
		sizeof(WNDCLASSEX), 
		CS_OWNDC | CS_HREDRAW | CS_VREDRAW, 
		WndProc, 
		0, 0, 
		hInstance, 
		NULL, NULL, 
		(HBRUSH)(COLOR_WINDOW + 1), 
		NULL,
		WINDOW_CLASS, 
		NULL
	};
    RECT R = {0, 0, width, height};
	s_pixel = pixel;
	s_width = w;
	s_height = h;
	if(!RegisterClassEx(&wcex))
    {
		return;
    }
	AdjustWindowRect(&R, WINDOW_STYLE, FALSE);
	if (!(hAppWnd = CreateWindow(WINDOW_CLASS, 
		WINDOW_TITLE, 
		WINDOW_STYLE, 
		CW_USEDEFAULT,
		0, R.right - R.left, R.bottom - R.top, 
		GetForegroundWindow() , NULL, hInstance, NULL)))
	{
        return;
    }
	ShowWindow(hAppWnd, SW_SHOW);
	UpdateWindow(hAppWnd);
	while (1)
	{
		MSG msg;
		if (1)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else 
			{
				draw(hAppWnd, width, height);
				continue;
			}
			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			if(!GetMessage(&msg, 0, 0, 0))
			{
				break;
			}
			if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			draw(hAppWnd, width, height);
		}
	}
	return;
}

#endif

//---------------------------------------------

unsigned char *PixelCreate(int width, int height)
{
	return (unsigned char *)malloc(width * height * 3);
}

void PixelDelete(unsigned char *pixel)
{
	free(pixel);
}

void PixelSetPixel(unsigned char *pixel, int width, int height, 
	int x, int y, unsigned int color)
{
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
		return;
	}
	else
	{
		unsigned char b = color;
		unsigned char g = color >> 8;
		unsigned char r = color >> 16;
		pixel += (height - y - 1) * width * 3 + x * 3;
		*(pixel++) = r;
		*(pixel++) = b;
		*(pixel++) = g;
	}
}

void PixelClearColor(unsigned char *pixel, int width, int height, 
	unsigned int color)
{
	int x, y;
	unsigned char b = color;
	unsigned char g = color >> 8;
	unsigned char r = color >> 16;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			*(pixel++) = r;
			*(pixel++) = b;
			*(pixel++) = g;
		}
	}
}

void PixelBlt(unsigned char *dest, int width, int height, 
			  unsigned char *src, int sw, int sh,
			  int clipx, int clipy, int clipw, int cliph)
{
	int x, y;
	for (y = 0; y < sh; y++)
	{
		unsigned char *psrc = src + y * sw * 3;
		unsigned char *pdest = dest + 
			(height - ((sh - y) + clipy)) * width * 3 + 
			(clipx + 0) * 3;
		for (x = 0; x < sw; x++)
		{
			if (x < clipx || x >= (clipx + clipw) || 
			    (sh - y) < clipy || (sh - y) >= (clipy + cliph))
			{
				continue;
			}
			*(pdest++) = *(psrc++);
			*(pdest++) = *(psrc++);
			*(pdest++) = *(psrc++);
		}
	}
}

void PixelFillMask(unsigned char *pixel, int width, int height)
{
	int x, y;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			unsigned char gray;
			gray = ((8 - y) % 8) * 32;
			gray &= 0xFF;
			*(pixel++) = gray;
			*(pixel++) = gray;
			*(pixel++) = gray;
		}
	}	
}

int PixelDumpFile(unsigned char *pixel, int width, int height, 
	char *filename)
{
	FILE *fp = NULL;
	//43200
	//43200 + 54 + 2 = 43256
	unsigned int imagesize = width * height * 3;
	unsigned int filesize = imagesize + 56;
	unsigned char header[54] = {0};
	header[0] = 0x42; // 'B'
	header[1] = 0x4D; // 'M'
	header[2] = filesize; // bytes num
	header[3] = filesize >> 8; //
	header[4] = filesize >> 16; //
	header[5] = filesize >> 24; //
	//printf("0x%0x, 0x%0x, 0x%0x, 0x%0x\n", header[2], header[3], header[4], header[5]);
	header[8 + 2] = 0x36; // data offset = 54
	header[8 + 6] = 0x28;
	header[16 + 2] = width;
	header[16 + 3] = width >> 8;
	header[16 + 4] = width >> 16;
	header[16 + 5] = width >> 24;
	header[16 + 6] = height;
	header[16 + 7] = height >> 8;
	header[24 + 0] = height >> 16;
	header[24 + 1] = height >> 24;
	header[24 + 2] = 0x01; // biPlane = 1
	header[24 + 4] = 0x18; // biBitCount = 24
	header[32 + 2] = imagesize; // biSizeImage
	header[32 + 3] = imagesize >> 8; 
	header[32 + 4] = imagesize >> 16;
	header[32 + 5] = imagesize >> 24;
	header[32 + 6] = 0x12; // biXPelsMeter = B12H
	header[32 + 7] = 0x0B;
	header[40 + 2] = 0x12; // biYPelsMeter = B12H
	header[40 + 3] = 0x0B;
	if(NULL == (fp = fopen(filename, "wb+")))
	{
		fprintf(stderr, "open file error\n");
		return 1;
	}
	fwrite(header, sizeof(header), 1, fp);
	fwrite(pixel, width * height * 3, 1, fp);	
	fclose(fp);
	return 0;
}

#if TEST_PIXEL

void test1(void)
{
	int w = 800;
	int h = 480;
	int i;
	unsigned char *pixel = PixelCreate(w, h);
	//PixelFillMask(pixel, w, h);
	PixelClearColor(pixel, w, h, 0xCCCCCC);
	for (i = 0; i < 40000; i++)
	{
		PixelSetPixel(pixel, w, h, i, i, 0xff0000);
	}
	PixelDumpFile(pixel, w, h, "output.bmp");
	testwin(pixel, w, h, w, h);
	PixelDelete(pixel);
}

void test2(void)
{
	int w = 120;
	int h = 120;
	unsigned char *pixel = head_bmp;
	PixelDumpFile(pixel, w, h, "output.bmp");
	testwin(pixel, w, h, w, h);
}

void test3(void)
{
	int w = 180;
	int h = 240;
	unsigned char *pixel = PixelCreate(w, h);
	PixelClearColor(pixel, w, h, 0xCCCCCC);
	PixelBlt(pixel, w, h, head_bmp, 120, 120,
		10, 30, 120, 60);
	PixelDumpFile(pixel, w, h, "output.bmp");
	testwin(pixel, w, h, w, h);
	PixelDelete(pixel);
}

int main(int argc, char **argv)
{
	test3();
	return 0;
}

#endif /* TEST_PIXEL */



