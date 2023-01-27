CC = gcc
LD = gcc

WARNS = -Wall -Wextra -Werror -pedantic -pedantic-errors
CFLAGS = -std=c99 -O3 ${WARNS}
LFLAGS = -s

SRC = src
OBJ = obj
BIN = bin

OBJ_LIST = \
	${OBJ}/ckc.o \
	${OBJ}/token.o \
	${OBJ}/ckclib.o \
	${OBJ}/file.o \
	${OBJ}/lexer.o

EXEC = ${BIN}/ckc

.PHONY: all clean

all: ${EXEC}

clean:
	rm -r ${BIN}/*
	rm -r ${OBJ}/*

${OBJ}:
	mkdir ${OBJ}

${BIN}:
	mkdir ${BIN}

${EXEC}: ${OBJ_LIST} | ${BIN}
	${LD} -o $@ ${OBJ_LIST} ${LFLAGS}

${OBJ}/%.o: ${SRC}/%.c | ${OBJ}
	${CC} ${CFLAGS} -c $< -o $@
