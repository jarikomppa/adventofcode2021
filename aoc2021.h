#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tokenizer.h"

#define FAIL printf("[fail on line %d]", __LINE__)
#define SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x)))
#define SMOOTHERSTEP(x) ((x) * (x) * (x) * ((x) * ((x) * 6 - 15) + 10))

double scale(double x, double xmin, double xmax, double dmin, double dmax);
int rainbow(double pos);
int rainbow_hilight(double pos);
int rainbow_shadow(double pos);

extern unsigned int framebuffer[];
extern int do_blend;

void setupgif(int motionblurframes, const char* fn);
void nextframe(int ofs = 0, int commit = 1);
void endgif(int endframes = 0);
void clear();
void drawrect(int x0, int y0, int x1, int y1, unsigned int color);
void drawbox(int x0, int y0, int w, int h, unsigned int color);
void drawtri(double x0, double y0, double x1, double y1, double x2, double y2, unsigned int color);
void drawline(double x0, double y0, double x1, double y1, double w, unsigned int color);
void drawarrow(double x0, double y0, double x1, double y1, double w, unsigned int color);
void drawarrow(double start, double end, double x0, double y0, double x1, double y1, double w, unsigned int color);
void drawcircle(double x0, double y0, double r, unsigned int color);
void drawmanhattan(double x0, double y0, double d, unsigned int color);
void drawpie(double x0, double y0, double r, double v0, double v1, double dragout, unsigned int color);
void drawsquicle(double x0, double y0, double r, double d, unsigned int color);
void drawstring(const char* aString, int aX, int aY, int aColor);
void drawstring(const char* aString, int aX, int aY, int n, int aColor);
void drawstringf( int aX, int aY, int n, int aColor, const char* fmt, ...);
void drawchar(int aChar, int aX, int aY, int n, int aColor);
double catmullrom(double t, double p0, double p1, double p2, double p3);
double perlin2d(double x, double y, double freq, int depth);
void decfb();

void graphbar(double x0, double y0, double x1, double y1, double* data, int count, double minval, double maxval);
void graphline(double x0, double y0, double x1, double y1, double* data, int count, double minval, double maxval);
void graphpie(double x0, double y0, double x1, double y1, double* data, int count, double minval, double maxval);
void graphdot(double x0, double y0, double x1, double y1, double* data, int count, double minval, double maxval);

long long getticks();

int isprime(long long val);
char* load(const char* fname, int* len = 0);
void loadgrid(const char* fn, char*& grid, int& gridx, int& gridy);


namespace day_1 { void d(); }
namespace day_2 { void d(); }
namespace day_3 { void d(); }
namespace day_4 { void d(); }
namespace day_5 { void d(); }
namespace day_6 { void d(); }
namespace day_7 { void d(); }
namespace day_8 { void d(); }
namespace day_9 { void d(); }
namespace day_10 { void d(); }
namespace day_11 { void d(); }
namespace day_12 { void d(); }
namespace day_13 { void d(); }
namespace day_14 { void d(); }
namespace day_15 { void d(); }
namespace day_16 { void d(); }
namespace day_17 { void d(); }
namespace day_18 { void d(); }
namespace day_19 { void d(); }
namespace day_20 { void d(); }
namespace day_21 { void d(); }
namespace day_22 { void d(); }
namespace day_23 { void d(); }
namespace day_24 { void d(); }
namespace day_25 { void d(); }
