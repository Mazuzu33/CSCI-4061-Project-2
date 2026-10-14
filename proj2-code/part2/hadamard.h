// SPDX-FileCopyrightText: 2026 John Kolb <jhkolb@umn.edu>
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef HADAMARD_H
#define HADAMARD_H

#include <Python.h>

/*
 * Reads in matrix contents from a binary file
 * Saves the dimensions of the matrix at pointer locations specified by the caller
 * Returns a pointer to a memory region (either shared memory or on the
 * heap) containing matrix elements
 *
 * 'file_name': Name of the binary file to read from
 * 'n_rows': Pointer to unsigned to overwrite with number of matrix rows
 * 'n_cols': Pointer to unsigned to overwrite with matrix columns
 * 'sh_mem': If nonzero, matrix is saved to shared memory segment. Otherwise, it is
 *           placed on the heap.
 * Returns a pointer to memory containing matrix elements or NULL on failure
 */
int *read_matrix_from_file(const char *file_name, unsigned *n_rows, unsigned *n_cols, int sh_mem);

/*
 * Prints every element to standard out. Each row goes on its own line
 * and each element is separated by a comma.
 *
 * 'mat': Matrix data to print out
 * 'n_rows': Number of rows in the matrix
 * 'n_cols': Number of columns in the matrix
 */
void print_matrix(const int *mat, unsigned n_rows, unsigned n_cols);

// Computes the minimum of two unsigned values and returns it
unsigned min(unsigned a, unsigned b);

// Compute hadamard product in parallel with child processes coordinating via a pipe
PyObject *hadamard_pipe(PyObject *self, PyObject *args);

// Compute hadamard product in parallel with child processes coordinating via shared memory
PyObject *hadamard_shmem(PyObject *self, PyObject *args);

// Compute hadamard product in parallel with threads
PyObject *hadamard_threads(PyObject *self, PyObject *args);

#endif    // HADAMARD_H
