CC=gcc
CFLAGS=-O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LEX=flex
YACC=bison

OBJS=parser.tab.o lex.yy.o ast.o eval.o scope.o main.o

all: minic

parser.tab.c parser.tab.h: parser.y
	$(YACC) -d -v parser.y

lex.yy.c: lexer.l parser.tab.h
	$(LEX) lexer.l

minic: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f minic *.o parser.tab.* lex.yy.c parser.output
