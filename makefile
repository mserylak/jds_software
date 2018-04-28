all:	headerjds jds2info jdsadd jds2fil

headerjds:	headerjds.c
	gcc -Wall -o headerjds headerjds.c jds_tools.c -lm

jds2info:	jds2info.c
	gcc -Wall -o jds2info jds2info.c jds_tools.c -lm

jdsadd:	jdsadd.c
	gcc -Wall -o jdsadd jdsadd.c jds_tools.c -lm

jds2fil:	jds2fil.c
	gcc -Wall -o jds2fil jds2fil.c jds_tools.c -lm

clean:	all
	rm -f headerjds jds2info jdsadd jds2fil
