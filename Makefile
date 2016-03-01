all:
	bison -d language.y
	flex language.l
	gcc -o language lex.yy.c language.tab.c language.c

clean:
	rm language
	rm language.tab.*
	rm lex.yy.c
