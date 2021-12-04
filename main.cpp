#include "aoc2021.h"

void giftest()
{
	setupgif(2, "test.gif");

	for (int i = 0; i < 200; i++)
	{
		clear();
		drawrect(
			(int)(sin((i) * 0.0293847) * 400 + 512),
			(int)(sin((i + 20) * 0.0543563) * 400 + 512),
			(int)(sin((i + 40) * 0.0245838) * 400 + 512),
			(int)(sin((i + 60) * 0.0498257) * 400 + 512),
			0xffeebbaa);
		drawbox(
			(int)(sin((i) * 0.0393847) * 400 + 512),
			(int)(sin((i + 20) * 0.0443563) * 400 + 512),
			(int)(sin((i + 40) * 0.0345838) * 400 + 512),
			(int)(sin((i + 60) * 0.0598257) * 400 + 512),
			0xffaabbee);
		drawtri(
			(int)(sin((i) * 0.0293847) * 400 + 512),
			(int)(sin((i + 20) * 0.0543563) * 400 + 512),
			(int)(sin((i + 40) * 0.0245838) * 400 + 512),
			(int)(sin((i + 60) * 0.0498257) * 400 + 512),
			(int)(sin((i) * 0.0393847) * 400 + 512),
			(int)(sin((i + 20) * 0.0443563) * 400 + 512),
			0xff00ff00);
		drawarrow(10, 10,
			sin((i + 20) * 0.0543563) * 400 + 512,
			sin((i + 40) * 0.0245838) * 400 + 512,
			sin((i) * 0.0393847) * 10 + 12, 0xffff0000);

		drawcircle(
			512 + sin((i + 60) * 0.0498257) * 400,
			512 + sin((i + 20) * 0.0443563) * 400,
			40 + sin(i * 0.0238467) * 20, 0xff0000ff);

		for (int k = 0; k < 20; k++)
			drawline(
				k * (1024 / 20),
				sin((i + k) * 0.239847) * 50 + 512,
				(k + 1) * (1024 / 20),
				sin((i + k + 1) * 0.239847) * 50 + 512,
				2, 0x7f7f7f7f);
		drawstring("Testing testing 123", 0, 0, 0xffffffff);
		drawstring("Whaat", 0, 12, 3, 0xffffffff);
		nextframe();
	}

	endgif();

}

int numprimes(long long pv1, long long pv2, int c)
{
	int count = 0;
	for (int i = 0; i < c; i++)
	{
		long long v = pv1 + pv2;
		if (isprime(v))// printf("P ");
			//printf("%llu\n", v);
			count++;
		pv2 = pv1;
		pv1 = v;
	}
	return count;
}

void primetest()
{
	setupgif(4, "fibprimes.gif");
	clear();
	for (int c = 0; c < 50; c++)
	{
		for (int i = 1; i < 31; i++)
		{
			drawstringf(16 + i * 32, 18, 2, 0xffffffff, "%2d", i);
			drawstringf(16, 18 + i * 32, 2, 0xffffffff, "%2d", i);
			for (int j = 1; j < 31; j++)
			{
				int n = numprimes(i, j, c);
				drawcircle(32 + 32 * i, 32 + 32 * j, n, 0xffaabbcc);
				printf("%2d ", n);
			}
			printf("\n");
		}
		nextframe();
	}
	for (int c = 0; c < 10; c++)
		nextframe();
	clear();
	for (int c = 0; c < 10; c++)
		nextframe();

	endgif();
}

void testparse()
{
	struct Tempdata
	{
		int temp, year;
		Tempdata() { temp = 0; year = 0; }
	} tempdata[1024];
	int idx = 0;
	int maxdata = 0;

	setupgif(1, "tempdata.gif");
	char* data = load("fie00145622.csv");
	Tokenizer lines;
	lines.init(data, "\r\n");
	int lastyear = 1958;
	for (int l = 0; l < lines.count(); l++)
	{
		if (lines.get(l)[0])
		{
			Tokenizer tok;
			tok.init(lines.get(l), ",");
			char temp[16];
			memcpy(temp, tok.get(1) + 1, 4);
			temp[4] = 0;
			int year = atoi(temp);
			if (year == lastyear)
			{
				year = 0;
			}
			else
			{
				lastyear = year;
			}
			int avgtemp = 0;
			int skip = 0;
			if (tok.get(10)[0])
			{
				memcpy(temp, tok.get(10) + 1, 5);
				temp[5] = 0;
				avgtemp = atoi(temp);
			}
			else
			{
				if (tok.get(12)[0] && tok.get(14)[0])
				{
					memcpy(temp, tok.get(12) + 1, 5);
					temp[5] = 0;
					int mintemp = atoi(temp);
					memcpy(temp, tok.get(14) + 1, 5);
					temp[5] = 0;
					int maxtemp = atoi(temp);
					avgtemp = (mintemp + maxtemp) / 2;
				}
				else
				{
					skip = 1;
				}
			}
			if (!skip)
			{
				printf("%d %d\n", year, avgtemp);
				tempdata[idx].year = year;
				tempdata[idx].temp = avgtemp;
				idx = (idx + 1) % 1024;
				maxdata++;
				if (maxdata > 1023) maxdata = 1023;
				static int skipper = 0;
				skipper++;
				if (skipper == 31)
				{
					skipper = 0;
					clear();
					double avg = 0;
					for (int i = 0; i < 1024; i++)
					{
						if (tempdata[(i + 1024 + idx) % 1024].year)
						{
							drawstringf(i, 64, 3, 0xffffffff, "%d", tempdata[(i + 1024 + idx) % 1024].year);
							drawrect(i, 0, i + 1, 1024, 0xcccccccc);
						}
						drawrect(i, 512, i + 1, 512 - tempdata[(i + 1024 + idx) % 1024].temp, tempdata[(i + 1024 + idx) % 1024].temp < 0 ? 0xffff0000:0xff0000ff);
						if (i < maxdata)
							avg += tempdata[i].temp;
					}
					avg /= maxdata;
					drawrect(0, (int)(512 - avg), 1024, (int)(512 - avg + 1), 0x80808080);
					nextframe();
				}
			}

		}
	}
	endgif();
	return;
}

int main(int parc, char** pars)
{
	return 0;
}
