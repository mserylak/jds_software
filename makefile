all:	headerjds jds2info jdsadd jds2fil

headerjds:	headerjds.c
	gcc -Wall -o headerjds headerjds.c jdstools.c -lm

jds2info:	jds2info.c
	gcc -Wall -o jds2info jds2info.c jdstools.c -lm

jdsadd:	jdsadd.c
	gcc -Wall -o jdsadd jdsadd.c jdstools.c -lm

jds2fil:	jds2fil.c
	gcc -Wall -o jds2fil jds2fil.c jdstools.c -lm

clean:	all
	rm -f headerjds jds2info jdsadd jds2fil
