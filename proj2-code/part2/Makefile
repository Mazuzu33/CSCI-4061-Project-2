# SPDX-FileCopyrightText: John Kolb <jhkolb@umn.edu>
# SPDX-License-Identifier: GPL-3.0-or-later
SHELL = /bin/bash
ARCH = $(shell uname -m)

.PHONY: clean test clean-tests zip

hadamard.cpython-310-$(ARCH)-linux-gnu.so: hadamard_pipe.c hadamard_shmem.c hadamard_threads.c
	pip install -e . -qq

ifdef testnum
test: hadamard.cpython-310-$(ARCH)-linux-gnu.so
	testius test_cases/tests.json -v -n $(testnum)
else
test: hadamard.cpython-310-$(ARCH)-linux-gnu.so
	testius test_cases/tests.json
endif

clean:
	rm -rf build hadamard.egg-info *.so

clean-tests:
	rm -rf test_results

zip:
	@echo "ERROR: You cannot run 'make zip' from the part2 subdirectory. Change to the main proj2-code directory and run 'make zip' there."
