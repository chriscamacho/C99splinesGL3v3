
APPNAME:=$(shell basename `pwd`)

INSTR:= -fsanitize=address,leak,undefined,pointer-compare,pointer-subtract
INSTR+= -fno-omit-frame-pointer

LDFLAGS:= -lGL -lm -lglfw

CFLAGS:= -Iinclude -Icglm/include -Wall -Wfatal-errors -std=c99


SRC:=$(wildcard src/*.c)
OBJ:=$(SRC:src/%.c=.build/%.o)
INC:=$(wildcard include/*.h)

CC=gcc

all: debug

$(APPNAME): $(OBJ)
	$(CC) $(OBJ) -o $(APPNAME) $(LDFLAGS)

$(OBJ): .build/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: debug release inst

debug: CFLAGS+= -g
debug: 
	@echo "*** made DEBUG target ***"

release: CFLAGS+= -O3
release: 
	@echo "*** made RELEASE target ***"

inst: CFLAGS+= -g $(INSTR)
inst: LDFLAGS+= $(INSTR)
inst: 
	@echo "*** made INSTRUMENTATION target ***"

release: CFLAGS+= -Ofast

debug release inst: $(APPNAME)

.PHONY:	clean
clean:
	rm .build/* -f
	touch .build/.dummy
	rm $(APPNAME) -f

style: $(SRC) $(INC)
	uncrustify --replace --no-backup -c tools/crusty.cfg src/*.c include/*.h 
