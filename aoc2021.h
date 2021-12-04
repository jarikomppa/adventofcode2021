#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tokenizer.h"

extern unsigned int framebuffer[];

void setupgif(int motionblurframes, const char* fn);
void nextframe(int ofs = 0);
void endgif();
void clear();
void drawrect(int x0, int y0, int x1, int y1, unsigned int color);
void drawbox(int x0, int y0, int w, int h, unsigned int color);
void drawtri(double x0, double y0, double x1, double y1, double x2, double y2, unsigned int color);
void drawline(double x0, double y0, double x1, double y1, double w, unsigned int color);
void drawarrow(double x0, double y0, double x1, double y1, double w, unsigned int color);
void drawcircle(double x0, double y0, double r, unsigned int color);
void drawstring(const char* aString, int aX, int aY, int aColor);
void drawstring(const char* aString, int aX, int aY, int n, int aColor);
void drawstringf( int aX, int aY, int n, int aColor, const char* fmt, ...);


long long getticks();

int isprime(long long val);
char* load(const char* fname, int* len = 0);

