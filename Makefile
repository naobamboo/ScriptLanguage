all:
	bison -d -v language.y
	flex language.l
	gcc -g -O0 -o language lex.yy.c language.tab.c language.c

clean:
	rm language
	rm language.tab.*
	rm lex.yy.c
