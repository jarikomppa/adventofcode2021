#include "aoc2021.h"
#include "gif.h"
#include <chrono>
#include <stdarg.h>

unsigned int framebuffer[1024 * 1024];
GifWriter gifwriter;
unsigned int* framedata;
int frames;
int frameidx;
int do_blend = 0;

int blend(int dst, int src)
{
	double alpha = ((src >> 24) & 0xff) / 255.0;
	double r0 = ((dst >> 0) & 0xff) / 255.0;
	double g0 = ((dst >> 8) & 0xff) / 255.0;
	double b0 = ((dst >> 16) & 0xff) / 255.0;
	double r1 = ((src >> 0) & 0xff) / 255.0;
	double g1 = ((src >> 8) & 0xff) / 255.0;
	double b1 = ((src >> 16) & 0xff) / 255.0;
	double r = (r0 * (1 - alpha)) + r1 * alpha;
	double g = (g0 * (1 - alpha)) + g1 * alpha;
	double b = (b0 * (1 - alpha)) + b1 * alpha;
	return ((int)(r * 255) << 0) | ((int)(g * 255) << 8) | ((int)(b * 255) << 16);
}

void blendwrite(int ofs, int c)
{
	if (do_blend)
		framebuffer[ofs] = blend(framebuffer[ofs], c);
	else
		framebuffer[ofs] = framebuffer[ofs] = c;
}

double scale(double x, double xmin, double xmax, double dmin, double dmax)
{
	return ((x - xmin) / (xmax - xmin)) * (dmax - dmin) + dmin;
}


void setupgif(int motionblurframes, const char* fn)
{
	GifBegin(&gifwriter, fn, 512, 512, 3);
	frames = motionblurframes;
	framedata = new unsigned int[512 * 512 * frames];
	frameidx = 0;
	memset(framedata, 0, 512 * 512 * frames * sizeof(int));
}

unsigned int frame[512 * 512];
void nextframe(int ofs, int commit)
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
	if (commit)
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
			blendwrite(i * 1024 + j, color);
}

void drawrect(double x0, double y0, double x1, double y1, unsigned int color)
{
	drawrect((int)x0, (int)y0, (int)x1, (int)y1, color);
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
		blendwrite(y0 * 1024 + j, color);
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
	if (x0 < 0 && x1 < 0) return;
	if (y0 < 0 && y1 < 0) return;
	if (x0 > 1023 && x1 > 1023) return;
	if (y0 > 1023 && y1 > 1023) return;

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
	drawline(x0, y0, x1 - ux * w * 3*4, y1 - uy * w * 3*4, w, color);
	drawtri(
		x1, y1,
		x1 + (nx - ux * 3) * w * 4,
		y1 + (ny - uy * 3) * w * 4,
		x1 + (-nx - ux * 3) * w * 4,
		y1 + (-ny - uy * 3) * w * 4,
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
	
	drawline(x0, y0, x1 - ux * w * 2, y1 - uy * w * 2, w, color);
	drawtri(
		x1, y1,
		x1 + (nx - ux * 3) * w * 4,
		y1 + (ny - uy * 3) * w * 4,
		x1 + (-nx - ux * 3) * w * 4,
		y1 + (-ny - uy * 3) * w * 4,
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

void drawpie(double x0, double y0, double r, double v0, double v1, double dragout, unsigned int color)
{
	double px1 = x0;
	double py1 = y0 + r;
	double ofs = 2 * 3.141592653589 * v0;
	double range = v1 - v0;
	double nx = (double)sin(ofs + (range * 0.5) * 2 * 3.141592653589);
	double ny = (double)cos(ofs + (range * 0.5) * 2 * 3.141592653589);
	x0 += nx * dragout;
	y0 += ny * dragout;
	int iters = (int)(r * 0.5 * range);
	if (iters < 16) iters = 16;
	for (int i = 0; i < iters + 1; i++)
	{
		double x1 = x0 + (double)sin(ofs + range * 2 * 3.141592653589 * ((i) / (double)iters)) * r;
		double y1 = y0 + (double)cos(ofs + range * 2 * 3.141592653589 * ((i) / (double)iters)) * r;
		if (i)
			drawtri(
				x0, 
				y0, 
				px1 + nx * dragout,
				py1 + ny * dragout,
				x1 + nx * dragout,
				y1 + ny * dragout,
				color);
		px1 = x1;
		py1 = y1;
	}
}


void drawsquicle(double x0, double y0, double r, double d, unsigned int color)
{
	double px1 = x0;
	double py1 = y0 + r;
	int iters = (int)r / 2;
	if (iters < 16) iters = 16;
	for (int i = 0; i < iters + 1; i++)
	{
		double x1c = (double)sin(3.141592653589 * 2 * ((i + 1) / (double)iters));
		double y1c = (double)cos(3.141592653589 * 2 * ((i + 1) / (double)iters));
		double x1s = 0;
		double y1s = 0;
		if (abs(x1c) > abs(y1c))
		{
			// x major
			// y = r / x
			double scale = abs(1.0 / x1c);
			x1s = x1c * scale;
			y1s = y1c * scale;
		}
		else
		{
			// y major
			// x = r / y
			double scale = abs(1.0 / y1c);
			x1s = x1c * scale;
			y1s = y1c * scale;
		}
		double x1 = x0 + (x1c + (x1s - x1c) * d) * r;
		double y1 = y0 + (y1c + (y1s - y1c) * d) * r;
		drawtri(x0, y0, px1, py1, x1, y1, color);
		px1 = x1;
		py1 = y1;
	}
}

void graphbar(double x0, double y0, double x1, double y1, double* data, int count, double minval, double maxval)
{
	double w = x1 - x0;
	double h = y1 - y0;
	double cw = w / count;
	if (maxval < 0.001)
	{
		minval = 100000000;
		for (int i = 0; i < count; i++)
		{
			if (data[i] > maxval)
				maxval = data[i];
			if (data[i] < minval)
				minval = data[i];
		}
	}
	double range = maxval - minval;

	for (int i = 0; i < count; i++)
		drawrect(
			x0 + cw * i + cw / 2, 
			y0 + h * (1 - ((data[i] - minval) / range)), 
			x0 + cw * i + cw, 
			y1, 
			rainbow(i / (double)count));
}

void graphline(double x0, double y0, double x1, double y1, double* data, int count, double minval, double maxval)
{
	double w = x1 - x0;
	double h = y1 - y0;
	double cw = w / count;
	if (maxval < 0.001)
	{
		minval = 100000000;
		for (int i = 0; i < count; i++)
		{
			if (data[i] > maxval)
				maxval = data[i];
			if (data[i] < minval)
				minval = data[i];
		}
	}
	double range = maxval - minval;

	double px = 0, py = 0;
	for (int i = 0; i < count; i++)
	{
		if (i)
		drawline(
			x0 + cw * i + cw / 2,
			y0 + h * (1 - ((data[i] - minval) / range)),
			px,
			py,
			4,
			rainbow(i / (double)count));
		px = x0 + cw * i + cw / 2;
		py = y0 + h * (1 - ((data[i] - minval) / range));
	}
}

void graphpie(double x0, double y0, double x1, double y1, double* data, int count, double minval, double maxval)
{
	double x = (x0 + x1) / 2;
	double y = (y0 + y1) / 2;
	double w = x1 - x0;
	double h = y1 - y0;
	double r = ((w > h) ? h : w) / 2 - maxval - minval * count;
	double total = 0;
	for (int i = 0; i < count; i++)
		total += data[i];

	double ofs = 0;
	for (int i = 0; i < count; i++)
	{
		drawpie(x, y, r - maxval, ofs, ofs + data[i] / total, maxval, rainbow(i / (double)count));
		ofs += data[i] / total;
		maxval += minval;
	}
}

void graphdot(double x0, double y0, double x1, double y1, double* data, int count, double minval, double maxval)
{
	double w = x1 - x0;
	double h = y1 - y0;
	double cw = w / count;
	if (maxval < 0.001)
	{
		minval = 100000000;
		for (int i = 0; i < count; i++)
		{
			if (data[i] > maxval)
				maxval = data[i];
			if (data[i] < minval)
				minval = data[i];
		}
	}
	double range = maxval - minval;

	for (int i = 0; i < count; i++)
		drawcircle(
			x0 + cw * i + cw / 2,
			y0 + h * (1 - ((data[i] - minval) / range)),
			cw / 3,
			rainbow(i / (double)count));
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
				blendwrite((aY + i) * 1024 + aX + j, aColor);
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
		if ((aY + i) > 0 && (aY + i) < 1024)
		for (j = 0; j < 8*n; j++)
		{
			if ((aX + j) > 0 && (aX + j) < 1024)
			if (ispixel(aChar, j/n, i/n))
			{
				blendwrite((aY + i) * 1024 + aX + j, aColor);
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

void hsv2rgb(double h, double s, double v, double&r, double&g, double&b)
{
	double S, H, V, F, M, N, K;
	int   I;

	S = s;  
	H = h;  
	V = v;  

	if (S == 0.0) 
	{
		//Achromatic case, set level of grey
		r = V;
		g = V;
		b = V;
	}
	else 
	{
		// Determine levels of primary colours.
		if (H >= 1.0) 
		{
			H = 0.0;
		}
		else 
		{
			H = H * 6;
		} 
		I = (int)H;   // should be in the range 0..5 
		F = H - I;    // fractional part

		M = V * (1 - S);
		N = V * (1 - S * F);
		K = V * (1 - S * (1 - F));

		if (I == 0) { r = V; g = K; b = M; }
		if (I == 1) { r = N; g = V; b = M; }
		if (I == 2) { r = M; g = V; b = K; }
		if (I == 3) { r = M; g = N; b = V; }
		if (I == 4) { r = K; g = M; b = V; }
		if (I == 5) { r = V; g = M; b = N; }
	}
}

int rainbow(double pos)
{
	pos = (double)fmod(pos, 1);
	double red, green, blue;
	hsv2rgb(pos, 0.8f, 0.8f, red, green, blue);

	return ((int)(red * 0xff) << 0) | ((int)(green * 0xff) << 8) | ((int)(blue * 0xff) << 16);
}

int rainbow_hilight(double pos)
{
	pos = (double)fmod(pos, 1);
	double red, green, blue;
	hsv2rgb(pos, 0.6f, 1.0f, red, green, blue);

	return ((int)(red * 0xff) << 0) | ((int)(green * 0xff) << 8) | ((int)(blue * 0xff) << 16);
}

int rainbow_shadow(double pos)
{
	pos = (double)fmod(pos, 1);
	double red, green, blue;
	hsv2rgb(pos, 0.8f, 0.6f, red, green, blue);

	return ((int)(red * 0xff) << 0) | ((int)(green * 0xff) << 8) | ((int)(blue * 0xff) << 16);
}

double catmullrom(double t, double p0, double p1, double p2, double p3)
{
	return 0.5 * (
		(2 * p1) +
		(-p0 + p2) * t +
		(2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
		(-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t
		);
}

static const int  SEED = 1985;

static const unsigned char  HASH[] = {
	208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
	185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
	9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
	70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
	203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
	164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
	228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
	232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
	193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
	101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
	135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
	114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219
};

static int noise2(int x, int y)
{
	int  yindex = (y + SEED) % 256;
	if (yindex < 0)
		yindex += 256;
	int  xindex = (HASH[yindex] + x) % 256;
	if (xindex < 0)
		xindex += 256;
	const int  result = HASH[xindex];
	return result;
}

static double lin_inter(double x, double y, double s)
{
	return x + s * (y - x);
}

static double smooth_inter(double x, double y, double s)
{
	return lin_inter(x, y, s * s * (3 - 2 * s));
}

static double noise2d(double x, double y)
{
	const int  x_int = (int)floor(x);
	const int  y_int = (int)floor(y);
	const double  x_frac = x - x_int;
	const double  y_frac = y - y_int;
	const int  s = noise2(x_int, y_int);
	const int  t = noise2(x_int + 1, y_int);
	const int  u = noise2(x_int, y_int + 1);
	const int  v = noise2(x_int + 1, y_int + 1);
	const double  low = smooth_inter(s, t, x_frac);
	const double  high = smooth_inter(u, v, x_frac);
	const double  result = smooth_inter(low, high, y_frac);
	return result;
}

double perlin2d(double x, double y, double freq, int depth)
{
	double  xa = x * freq;
	double  ya = y * freq;
	double  amp = 1.0;
	double  fin = 0;
	double  div = 0.0;
	for (int i = 0; i < depth; i++)
	{
		div += 256 * amp;
		fin += noise2d(xa, ya) * amp;
		amp /= 2;
		xa *= 2;
		ya *= 2;
	}
	return fin / div;
}
