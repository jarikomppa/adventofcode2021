
for x in range(2,26):
    with open(f"d{x}.cpp", "w") as f:
        f.write(f"""
#include "aoc2021.h"


void d{x}_1()
{{
	Tokenizer lines;
	char* data = load("d{x}.txt");
	lines.init(data, "\\r\\n");
	if (!data)
	{{
		printf("File not found\\n");
		return;
	}}

	for (int l = 0; l < lines.count(); l++)
	{{
		if (lines.get(l)[0])
		{{
			Tokenizer tok;
			tok.init(lines.get(l), ",");
			// ..
		}}
	}}
	clear();
	drawrect(500, 500, 1024, 1014, 0xffccbbaa);
	nextframe();

}}

void d{x}_2()
{{
	Tokenizer lines;
	char* data = load("d{x}.txt");
	lines.init(data, "\\r\\n");
	if (!data)
	{{
		printf("File not found\\n");
		return;
	}}

	for (int l = 0; l < lines.count(); l++)
	{{
		if (lines.get(l)[0])
		{{
			Tokenizer tok;
			tok.init(lines.get(l), ",");
			// ..
		}}
	}}

	clear();
	drawrect(500, 500, 1024, 1014, 0xffccbbaa);
	nextframe();
}}

void d{x}()
{{
	setupgif(1, "d{x}.gif");
	d{x}_1();
	d{x}_2();
	endgif();
}}
""")