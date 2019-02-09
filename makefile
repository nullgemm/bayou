NAME = bayou
CC = gcc
FLAGS = -std=c99 -pedantic -g
FLAGS+= -Wall -Wno-unused-parameter -Wextra -Werror=vla -Werror
VALGRIND = --show-leak-kinds=all --track-origins=yes --leak-check=full

BIND = bin
OBJD = obj
SRCD = src
SUBD = sub
TESTD = tests

INCL = -I$(SRCD)
INCL+= -I$(SUBD)/testoasterror/src

FINAL = $(SRCD)/main.c

TESTS = $(TESTD)/main.c
TESTS+= $(SUBD)/testoasterror/src/testoasterror.c

SRCS = $(SRCD)/bayou.c
SRCS+= $(SRCD)/short.c

FINAL_OBJS:= $(patsubst %.c,$(OBJD)/%.o,$(FINAL))
SRCS_OBJS := $(patsubst %.c,$(OBJD)/%.o,$(SRCS))
TESTS_OBJS:= $(patsubst %.c,$(OBJD)/%.o,$(TESTS))

# aliases
.PHONY: final
final: $(BIND)/$(NAME)
tests: $(BIND)/tests

# generic compiling command
$(OBJD)/%.o: %.c
	@echo "building object $@"
	@mkdir -p $(@D)
	@$(CC) $(INCL) $(FLAGS) -c -o $@ $<

# final executable
$(BIND)/$(NAME): $(SRCS_OBJS) $(FINAL_OBJS)
	@echo "compiling executable $@"
	@mkdir -p $(@D)
	@$(CC) -o $@ $^ $(LINK)

run:
	@cd $(BIND) && ./$(NAME)

# tests executable
$(BIND)/tests: $(SRCS_OBJS) $(TESTS_OBJS)
	@echo "compiling tests"
	@mkdir -p $(@D)
	@$(CC) -o $@ $^ $(LINK)

check:
	@cd $(BIND) && ./tests

# tools
leakgrind: $(BIND)/$(NAME)
	@rm -f valgrind.log
	@cd $(BIND) && valgrind $(VALGRIND) 2> ../valgrind.log ./$(NAME)

leakgrindcheck: $(BIND)/tests
	@rm -f valgrind.log
	@cd $(BIND) && valgrind $(VALGRIND) 2> ../valgrind.log ./tests

clean:
	@echo "cleaning"
	@rm -rf $(BIND) $(OBJD) valgrind.log

github:
	@echo "sourcing submodules from https://github.com"
	@mv .github .gitmodules
	@git submodule sync
	@git submodule update --init --recursive --remote

gitea:
	@echo "sourcing submodules from https://git.cylgom.net"
	@mv .gitea .gitmodules
	@git submodule sync
	@git submodule update --init --recursive --remote
