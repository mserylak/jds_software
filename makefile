all:	headerjds jds2info jdsadd jds2rawfbk

headerjds:	headerjds.c
	gcc -Wall -o headerjds headerjds.c jdstools.c -lm

jds2info:	jds2info.c
	gcc -Wall -o jds2info jds2info.c jdstools.c -lm

jdsadd:	jdsadd.c
	gcc -Wall -o jdsadd jdsadd.c jdstools.c -lm

jds2rawfbk:	jds2rawfbk.c
	gcc -Wall -o jds2rawfbk jds2rawfbk.c jdstools.c -lm
