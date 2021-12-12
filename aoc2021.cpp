#include "aoc2021.h"
#include "gif.h"
#include <chrono>
#include <stdarg.h>

unsigned int framebuffer[1024 * 1024];
GifWriter gifwriter;
unsigned int* framedata;
int frames;
int frameidx;

void setupgif(int motionblurframes, const char* fn)
{
	GifBegin(&gifwriter, fn, 512, 512, 3);
	frames = motionblurframes;
	framedata = new unsigned int[512 * 512 * frames];
	frameidx = 0;
	memset(framedata, 0, 512 * 512 * frames * sizeof(int));
}

unsigned int frame[512 * 512];
void nextframe(int ofs)
{
	for (int i = 0; i < 512; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			int c =
				((framebuffer[(i * 2 + 0) * 1024 + (ofs + j * 2 + 0) % 1024] & 0xfcfcfcfc) >> 2) +
				((framebuffer[(i * 2 + 0) * 1024 + (ofs + j * 2 + 1) % 1024] & 0xfcfcfcfc) >> 2) +
				((framebuffer[(i * 2 + 1) * 1024 + (ofs + j * 2 + 0) % 1024] & 0xfcfcfcfc) >> 2) +
				((framebuffer[(i * 2 + 1) * 1024 + (ofs + j * 2 + 1) % 1024] & 0xfcfcfcfc) >> 2);
			framedata[512 * 512 * frameidx + i * 512 + j] = c;
		}
	}

	frameidx = (frameidx + 1) % frames;

	memset(frame, 0, 512 * 512 * sizeof(int));
	for (int i = 0; i < 512; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			unsigned int r = 0;
			unsigned int g = 0;
			unsigned int b = 0;
			for (int k = 0; k < frames; k++)
			{
				r += (framedata[512 * 512 * k + i * 512 + j] >> 0) & 0xff;
				g += (framedata[512 * 512 * k + i * 512 + j] >> 8) & 0xff;
				b += (framedata[512 * 512 * k + i * 512 + j] >> 16) & 0xff;
			}
			r /= frames;
			g /= frames;
			b /= frames;
			frame[i * 512 + j] = 0xff000000 | (b << 16) | (g << 8) | r;
		}
	}
	GifWriteFrame(&gifwriter, (const unsigned char*)frame, 512, 512, 3);
}

void endgif()
{
	GifEnd(&gifwriter);
}

void drawrect(int x0, int y0, int x1, int y1, unsigned int color)
{
	if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
	if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
	if (x0 < 0) x0 = 0;
	if (x1 > 1024) x1 = 1024;
	if (y0 < 0) y0 = 0;
	if (y1 > 1024) y1 = 1024;
	for (int i = y0; i < y1; i++)
		for (int j = x0; j < x1; j++)
			framebuffer[i * 1024 + j] = color;
}

void drawspan(int x0, int y0, int x1, unsigned int color)
{
	if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
	x1++;
	if (x0 < 0) x0 = 0;
	if (x1 > 1023) x1 = 1023;
	if (y0 < 0) y0 = 0;
	if (y0 > 1023) y0 = 1023;
	for (int j = x0; j < x1; j++)
		framebuffer[y0 * 1024 + j] = color;
}


void drawbox(int x0, int y0, int w, int h, unsigned int color)
{
	drawrect(x0, y0, x0 + w, y0 + h, color);
}

void clear()
{
	memset(framebuffer, 0, 1024 * 1024 * sizeof(int));
}

void drawtri(double x0, double y0, double x1, double y1, double x2, double y2, unsigned int color)
{
	struct vertex 
	{
		double x, y;
	} vtx[3];

	vtx[0].x = x0;
	vtx[0].y = (int)y0;
	vtx[1].x = x1;
	vtx[1].y = (int)y1;
	vtx[2].x = x2;
	vtx[2].y = (int)y2;
	
	int order[3];

	// find the lowest Y
	if (vtx[0].y < vtx[1].y)
	{
		if (vtx[0].y < vtx[2].y) { order[0] = 0; } else { order[0] = 2; }
	}
	else
	{
		if (vtx[1].y < vtx[2].y) { order[0] = 1; } else { order[0] = 2; }
	}

	// find the highest Y
	if (vtx[0].y > vtx[1].y)
	{
		if (vtx[0].y > vtx[2].y) { order[2] = 0; } else { order[2] = 2; }
	}
	else
	{
		if (vtx[1].y > vtx[2].y) { order[2] = 1; } else { order[2] = 2; }
	}

	// and the middle one is a matter of deduction
	order[1] = 3 - (order[0] + order[2]);

	if ((int)vtx[order[2]].y == (int)vtx[order[0]].y)
		return; // degenerate triangle
	
	double left = vtx[order[0]].x;
	double right = vtx[order[0]].x;
	double d0 = (vtx[order[1]].x - vtx[order[0]].x) / (vtx[order[1]].y - vtx[order[0]].y);
	double d1 = (vtx[order[2]].x - vtx[order[0]].x) / (vtx[order[2]].y - vtx[order[0]].y);

	if ((int)vtx[order[1]].y != (int)vtx[order[0]].y)
	{
		for (int y = (int)vtx[order[0]].y; y < (int)vtx[order[1]].y; y++)
		{
			drawspan((int)left, y, (int)right, color);
			left += d0;
			right += d1;
		}
		if ((int)vtx[order[2]].y == (int)vtx[order[1]].y)
			return; // flat bottom triagle
	}
	else
	{
		// flat top triangle
		left = vtx[order[1]].x;
		right = vtx[order[0]].x;
	}

	d0 = (vtx[order[2]].x - vtx[order[1]].x) / (vtx[order[2]].y - vtx[order[1]].y);

	for (int y = (int)vtx[order[1]].y; y < (int)vtx[order[2]].y; y++)
	{
		drawspan((int)left, y, (int)right, color);
		left += d0;
		right += d1;
	}
}

void drawline(double x0, double y0, double x1, double y1, double w, unsigned int color)
{
	double nx = y0 - y1;
	double ny = x1 - x0;
	double nd = (double)sqrt(nx * nx + ny * ny);
	if (nd < 1) return;
	nx /= nd;
	ny /= nd;
	nx *= w / 2;
	ny *= w / 2;
	drawtri(
		x0 + nx, y0 + ny,
		x1 + nx, y1 + ny,
		x0 - nx, y0 - ny,
		color);
	drawtri(
		x0 - nx, y0 - ny,
		x1 - nx, y1 - ny,
		x1 + nx, y1 + ny,
		color);
}

void drawarrow(double x0, double y0, double x1, double y1, double w, unsigned int color)
{
	double nx = y0 - y1;
	double ny = x1 - x0;
	double nd = (double)sqrt(nx * nx + ny * ny);
	if (nd < 1) return;
	nx /= nd;
	ny /= nd;
	double uy = y1 - y0;
	double ux = x1 - x0;
	double ud = (double)sqrt(ux * ux + uy * uy);
	ux /= ud;
	uy /= ud;
	drawline(x0, y0, x1 + ux * w * 0.5, y1 + uy * w * 0.5, w, color);
	drawline(
		x1,
		y1,
		x1 + (nx - ux) * w * 4,
		y1 + (ny - uy) * w * 4,
		w,
		color);
	drawline(
		x1,
		y1,
		x1 + (-nx - ux) * w * 4,
		y1 + (-ny - uy) * w * 4,
		w,
		color);
}

void drawarrow(double start, double end, double x0, double y0, double x1, double y1, double w, unsigned int color)
{
	double nx = y0 - y1;
	double ny = x1 - x0;
	double nd = (double)sqrt(nx * nx + ny * ny);
	if (nd < 1) return;
	nx /= nd;
	ny /= nd;

	double uy = y1 - y0;
	double ux = x1 - x0;
	double ud = (double)sqrt(ux * ux + uy * uy);
	ux /= ud;
	uy /= ud;

	x0 += ux * start;
	y0 += uy * start;
	x1 -= ux * end;
	y1 -= uy * end;
	
	drawline(x0, y0, x1 + ux * w * 0.5, y1 + uy * w * 0.5, w, color);
	drawline(
		x1,
		y1,
		x1 + (nx - ux) * w * 4,
		y1 + (ny - uy) * w * 4,
		w,
		color);
	drawline(
		x1,
		y1,
		x1 + (-nx - ux) * w * 4,
		y1 + (-ny - uy) * w * 4,
		w,
		color);
}

void drawcircle(double x0, double y0, double r, unsigned int color)
{
	double px1 = x0;
	double py1 = y0 + r;
	int iters = (int)r / 2;
	if (iters < 16) iters = 16;
	for (int i = 0; i < iters; i++)
	{
		double x1 = x0 + (double)sin(3.141592653589 * 2 * ((i + 1) / (double)iters)) * r;
		double y1 = y0 + (double)cos(3.141592653589 * 2 * ((i + 1) / (double)iters)) * r;
		drawtri(x0, y0, px1, py1, x1, y1, color);
		px1 = x1;
		py1 = y1;
	}
}

unsigned char TFX_AsciiFontdata[12 * 256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,// ' '
  0, 12, 30, 30, 30, 12, 12,  0, 12, 12,  0,  0,// '!'
  0,102,102,102, 36,  0,  0,  0,  0,  0,  0,  0,// '"'
  0, 54, 54,127, 54, 54, 54,127, 54, 54,  0,  0,// '#'
 12, 12, 62,  3,  3, 30, 48, 48, 31, 12, 12,  0,// '$'
  0,  0,  0, 35, 51, 24, 12,  6, 51, 49,  0,  0,// '%'
  0, 14, 27, 27, 14, 95,123, 51, 59,110,  0,  0,// '&'
  0, 12, 12, 12,  6,  0,  0,  0,  0,  0,  0,  0,// '''
  0, 48, 24, 12,  6,  6,  6, 12, 24, 48,  0,  0,// '('
  0,  6, 12, 24, 48, 48, 48, 24, 12,  6,  0,  0,// ')'
  0,  0,  0,102, 60,255, 60,102,  0,  0,  0,  0,// '*'
  0,  0,  0, 24, 24,126, 24, 24,  0,  0,  0,  0,// '+'
  0,  0,  0,  0,  0,  0,  0,  0, 28, 28,  6,  0,// ','
  0,  0,  0,  0,  0,127,  0,  0,  0,  0,  0,  0,// '-'
  0,  0,  0,  0,  0,  0,  0,  0, 28, 28,  0,  0,// '.'
  0,  0, 64, 96, 48, 24, 12,  6,  3,  1,  0,  0,// '/'
  0, 62, 99,115,123,107,111,103, 99, 62,  0,  0,// '0'
  0,  8, 12, 15, 12, 12, 12, 12, 12, 63,  0,  0,// '1'
  0, 30, 51, 51, 48, 24, 12,  6, 51, 63,  0,  0,// '2'
  0, 30, 51, 48, 48, 28, 48, 48, 51, 30,  0,  0,// '3'
  0, 48, 56, 60, 54, 51,127, 48, 48,120,  0,  0,// '4'
  0, 63,  3,  3,  3, 31, 48, 48, 51, 30,  0,  0,// '5'
  0, 28,  6,  3,  3, 31, 51, 51, 51, 30,  0,  0,// '6'
  0,127, 99, 99, 96, 48, 24, 12, 12, 12,  0,  0,// '7'
  0, 30, 51, 51, 55, 30, 59, 51, 51, 30,  0,  0,// '8'
  0, 30, 51, 51, 51, 62, 24, 24, 12, 14,  0,  0,// '9'
  0,  0,  0, 28, 28,  0,  0, 28, 28,  0,  0,  0,// ':'
  0,  0,  0, 28, 28,  0,  0, 28, 28, 24, 12,  0,// ';'
  0, 48, 24, 12,  6,  3,  6, 12, 24, 48,  0,  0,// '<'
  0,  0,  0,  0,126,  0,126,  0,  0,  0,  0,  0,// '='
  0,  6, 12, 24, 48, 96, 48, 24, 12,  6,  0,  0,// '>'
  0, 30, 51, 48, 24, 12, 12,  0, 12, 12,  0,  0,// '?'
  0, 62, 99, 99,123,123,123,  3,  3, 62,  0,  0,// '@'
  0, 12, 30, 51, 51, 51, 63, 51, 51, 51,  0,  0,// 'A'
  0, 63,102,102,102, 62,102,102,102, 63,  0,  0,// 'B'
  0, 60,102, 99,  3,  3,  3, 99,102, 60,  0,  0,// 'C'
  0, 31, 54,102,102,102,102,102, 54, 31,  0,  0,// 'D'
  0,127, 70,  6, 38, 62, 38,  6, 70,127,  0,  0,// 'E'
  0,127,102, 70, 38, 62, 38,  6,  6, 15,  0,  0,// 'F'
  0, 60,102, 99,  3,  3,115, 99,102,124,  0,  0,// 'G'
  0, 51, 51, 51, 51, 63, 51, 51, 51, 51,  0,  0,// 'H'
  0, 30, 12, 12, 12, 12, 12, 12, 12, 30,  0,  0,// 'I'
  0,120, 48, 48, 48, 48, 51, 51, 51, 30,  0,  0,// 'J'
  0,103,102, 54, 54, 30, 54, 54,102,103,  0,  0,// 'K'
  0, 15,  6,  6,  6,  6, 70,102,102,127,  0,  0,// 'L'
  0, 99,119,127,127,107, 99, 99, 99, 99,  0,  0,// 'M'
  0, 99, 99,103,111,127,123,115, 99, 99,  0,  0,// 'N'
  0, 28, 54, 99, 99, 99, 99, 99, 54, 28,  0,  0,// 'O'
  0, 63,102,102,102, 62,  6,  6,  6, 15,  0,  0,// 'P'
  0, 28, 54, 99, 99, 99,115,123, 62, 48,120,  0,// 'Q'
  0, 63,102,102,102, 62, 54,102,102,103,  0,  0,// 'R'
  0, 30, 51, 51,  3, 14, 24, 51, 51, 30,  0,  0,// 'S'
  0, 63, 45, 12, 12, 12, 12, 12, 12, 30,  0,  0,// 'T'
  0, 51, 51, 51, 51, 51, 51, 51, 51, 30,  0,  0,// 'U'
  0, 51, 51, 51, 51, 51, 51, 51, 30, 12,  0,  0,// 'V'
  0, 99, 99, 99, 99,107,107, 54, 54, 54,  0,  0,// 'W'
  0, 51, 51, 51, 30, 12, 30, 51, 51, 51,  0,  0,// 'X'
  0, 51, 51, 51, 51, 30, 12, 12, 12, 30,  0,  0,// 'Y'
  0,127,115, 25, 24, 12,  6, 70, 99,127,  0,  0,// 'Z'
  0, 60, 12, 12, 12, 12, 12, 12, 12, 60,  0,  0,// '['
  0,  0,  1,  3,  6, 12, 24, 48, 96, 64,  0,  0,// '\'
  0, 60, 48, 48, 48, 48, 48, 48, 48, 60,  0,  0,// ']'
  8, 28, 54, 99,  0,  0,  0,  0,  0,  0,  0,  0,// '^'
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,  0,// '_'
 12, 12, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,// '`'
  0,  0,  0,  0, 30, 48, 62, 51, 51,110,  0,  0,// 'a'
  0,  7,  6,  6, 62,102,102,102,102, 59,  0,  0,// 'b'
  0,  0,  0,  0, 30, 51,  3,  3, 51, 30,  0,  0,// 'c'
  0, 56, 48, 48, 62, 51, 51, 51, 51,110,  0,  0,// 'd'
  0,  0,  0,  0, 30, 51, 63,  3, 51, 30,  0,  0,// 'e'
  0, 28, 54,  6,  6, 31,  6,  6,  6, 15,  0,  0,// 'f'
  0,  0,  0,  0,110, 51, 51, 51, 62, 48, 51, 30,// 'g'
  0,  7,  6,  6, 54,110,102,102,102,103,  0,  0,// 'h'
  0, 24, 24,  0, 30, 24, 24, 24, 24,126,  0,  0,// 'i'
  0, 48, 48,  0, 60, 48, 48, 48, 48, 51, 51, 30,// 'j'
  0,  7,  6,  6,102, 54, 30, 54,102,103,  0,  0,// 'k'
  0, 30, 24, 24, 24, 24, 24, 24, 24,126,  0,  0,// 'l'
  0,  0,  0,  0, 63,107,107,107,107, 99,  0,  0,// 'm'
  0,  0,  0,  0, 31, 51, 51, 51, 51, 51,  0,  0,// 'n'
  0,  0,  0,  0, 30, 51, 51, 51, 51, 30,  0,  0,// 'o'
  0,  0,  0,  0, 59,102,102,102,102, 62,  6, 15,// 'p'
  0,  0,  0,  0,110, 51, 51, 51, 51, 62, 48,120,// 'q'
  0,  0,  0,  0, 55,118,110,  6,  6, 15,  0,  0,// 'r'
  0,  0,  0,  0, 30, 51,  6, 24, 51, 30,  0,  0,// 's'
  0,  0,  4,  6, 63,  6,  6,  6, 54, 28,  0,  0,// 't'
  0,  0,  0,  0, 51, 51, 51, 51, 51,110,  0,  0,// 'u'
  0,  0,  0,  0, 51, 51, 51, 51, 30, 12,  0,  0,// 'v'
  0,  0,  0,  0, 99, 99,107,107, 54, 54,  0,  0,// 'w'
  0,  0,  0,  0, 99, 54, 28, 28, 54, 99,  0,  0,// 'x'
  0,  0,  0,  0,102,102,102,102, 60, 48, 24, 15,// 'y'
  0,  0,  0,  0, 63, 49, 24,  6, 35, 63,  0,  0,// 'z'
  0, 56, 12, 12,  6,  3,  6, 12, 12, 56,  0,  0,// '{'
  0, 24, 24, 24, 24,  0, 24, 24, 24, 24,  0,  0,// '|'
  0,  7, 12, 12, 24, 48, 24, 12, 12,  7,  0,  0,// '}'
  0,206, 91,115,  0,  0,  0,  0,  0,  0,  0,  0,// '~'
};

// The pixel data is 8x12 bits, so each glyph is 12 bytes.
int ispixel(char ch, int x, int y)
{
	return (TFX_AsciiFontdata[(ch - 32) * 12 + y] & (1 << x)) != 0;
}

void drawchar(int aChar, int aX, int aY, int aColor)
{
	int i, j;
	for (i = 0; i < 12; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (ispixel(aChar, j, i))
			{
				framebuffer[(aY + i) * 1024 + aX + j] = aColor;
			}
		}
	}
}

void drawstring(const char* aString, int aX, int aY, int aColor)
{
	if (aY + 8 > 1024) return;
	while (*aString)
	{
		drawchar(*aString, aX, aY, aColor);
		aX += 8;
		if (aX + 8 > 1024)
			return;
		aString++;
	}
}

void drawstringf(int aX, int aY, int n, int aColor, const char* fmt, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, 255, fmt, args);
	va_end(args);
	drawstring(buffer, aX, aY, n, aColor);
}

void drawchar(int aChar, int aX, int aY, int n, int aColor)
{
	int i, j;
	for (i = 0; i < 12*n; i++)
	{
		for (j = 0; j < 8*n; j++)
		{
			if (ispixel(aChar, j/n, i/n))
			{
				framebuffer[(aY + i) * 1024 + aX + j] = aColor;
			}
		}
	}
}

void drawstring(const char* aString, int aX, int aY, int n, int aColor)
{
	while (*aString)
	{
		drawchar(*aString, aX, aY, n, aColor);
		aX += 8*n;
		if (aX + 8*n > 1024)
			return;
		aString++;
	}
}

int isprime(long long val)
{
	if (val < 0) return 0;
	if (!(val & 1)) return 0;
	long long maxdiv = (long long)sqrt((double)val);
	for (long long i = 3; i < maxdiv; i += 2)
	{
		if (!(val % i))
			return 0;
	}
	return 1;
}

long long getticks()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

char* load(const char* fname, int* len)
{
	int l;
	FILE* f;
	fopen_s(&f, fname, "rb");
	if (f == NULL)
	{
		printf("File %s not found\n", fname);
		exit(-1);
	}
	fseek(f, 0, SEEK_END);
	l = ftell(f);
	char* d = new char[l + 1];
	d[l] = 0;
	fseek(f, 0, SEEK_SET);
	fread(d, 1, l, f);
	if (len)
	{
		*len = l;
	}
	fclose(f);
	return d;
}
