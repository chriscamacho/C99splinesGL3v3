
APPNAME:=$(shell basename `pwd`)

CC=gcc

INSTR:= -fsanitize=address,leak,undefined,pointer-compare,pointer-subtract
INSTR+= -fno-omit-frame-pointer

LDFLAGS:= -lGL -lm -lglfw

CFLAGS:= -Iinclude -Isupport/include -Isupport/cglm/include -Wall -Wfatal-errors -std=c99

SRC:=$(wildcard src/*.c)
SRCS=$(wildcard support/src/*.c)
OBJ:=$(SRC:src/%.c=.build/%.o)
OBJS:=$(SRCS:support/src/%.c=.build/support/%.o)

all: debug


$(APPNAME): $(OBJ) $(OBJS)
	$(CC) $(OBJ) $(OBJS) -o $(APPNAME) $(LDFLAGS)

$(OBJ): .build/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS): .build/support/%.o : support/src/%.c
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
	rm .build/*.o -f
	rm .build/support/*.o -f
	rm $(APPNAME) -f

style: $(SRC) $(INC)
	uncrustify --replace --no-backup -c tools/crusty.cfg src/*.c include/*.h 
