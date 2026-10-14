# SPDX-FileCopyrightText: John Kolb <jhkolb@umn.edu>
# SPDX-License-Identifier: GPL-3.0-or-later
CFLAGS = -Wall -Werror -g
CC = gcc $(CFLAGS)
SHELL = /bin/bash
ARCH = $(shell uname -m)

.PHONY: clean test-setup test clean-tests zip

swish: swish.o string_vector.o swish_funcs.o swish_funcs_provided_$(ARCH).o
	$(CC) -o $@ $^

swish.o: swish.c
	$(CC) -c $<

string_vector.o: string_vector.c string_vector.h
	$(CC) -c $<

swish_funcs.o: swish_funcs.c swish_funcs.h
	$(CC) -c $<

clean:
	rm -f swish swish.o string_vector.o swish_funcs.o

test-setup:
	@rm -f out.txt

ifdef testnum
test: test-setup swish
	testius test_cases/tests.json -v -n $(testnum)
else
test: swish
	testius test_cases/tests.json
endif

clean-tests:
	rm -rf test_results out.txt

zip:
	@echo "ERROR: You cannot run 'make zip' from the part1 subdirectory. Change to the main proj2-code directory and run 'make zip' there."
