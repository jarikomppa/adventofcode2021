
for x in range(1,26):
    with open(f"d{x}.cpp", "w") as f:
        f.write(f"""
#include "aoc2021.h"

namespace day_{x}
{{

/*
	char* grid;
	int gridx, gridy;
	loadgrid("dXX.txt", grid, gridx, gridy);
*/

void parse()
{{
	Tokenizer lines;
	char* data = load("d{x}.txt");
	//char* data = load("t.txt");
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
			for (int i = 0; i < tok.count(); i++)
				printf("%d:%s ", i, tok.get(i));
			printf("\\n");
			// ..
		}}
	}}
}}

void d_1()
{{
}}

void d_2()
{{
}}

/*
void vis()
{{
	clear();
	drawrect(500, 500, 1024, 1014, 0xffccbbaa);
	nextframe();
}}
*/

void d()
{{
	setupgif(1, "d{x}.gif");
    parse();
	d_1();
	d_2();
	endgif();
}}

}}
""")
