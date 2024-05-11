PROGRAM		= xoshiro256ss
STALIB		= lib$(PROGRAM).a
DYNLIB		= lib$(PROGRAM).so

CC		= gcc
CFLAGS		= -O2 -march=native -Wall -Wextra
ASM		= nasm
ASMFLAGS	= -Ox -felf64 -w+all -w-reloc-rel-dword
LDLIBS		=
LDFLAGS		=
RM		= rm -fv

SRCS		= xoshiro256ss.c xoshiro256ss.s
OBJS		= $(SRCS:%=%.o)
DEPS		= $(SRCS:%=%.d)
INC		= .

.PHONY: all build debug
.DEFAULT_GOAL: all

debug: CFLAGS	+= -DDEBUG -g -Og -Wpedantic
debug: ASMFLAGS	+= -DDEBUG -g -Fdwarf
debug: build

all: build

build: $(STALIB) $(DYNLIB)

$(STALIB): $(OBJS)
	$(AR) rcs $@ $^

$(DYNLIB): $(OBJS)
	$(CC) -shared -o $@ $^ $(LDLIBS) $(LDFLAGS)

%.c.o: %.c
	$(CC) -MMD -MP $(CFLAGS) -o $@ -c $< -I$(INC)

%.s.o: %.s
	$(ASM) $(ASMFLAGS) -o $@ $<


.PHONY: clean dist-clean
clean: test-clean
	$(RM) *.o *.d

dist-clean: clean
	$(RM) $(STALIB) $(DYNLIB)


TEST		= test
export

.PHONY: test test-build test-clean
test-build: build
	cd $(TEST) && make build

test: test-build
	cd $(TEST) && make test

test-clean:
	cd $(TEST) && make clean

