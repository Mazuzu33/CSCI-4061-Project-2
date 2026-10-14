#define _GNU_SOURCE

#include <Python.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hadamard.h"

/*
 * Represents arguments for each thread
 * 'a': Pointer to first matrix operand
 * 'b': Pointer to second matrix operand
 * 'c': Pointer to result matrix
 * 'start': Index (in flattened representation) of first element of matrix portion
 * 'end': Index (in flattened representation) of second element of matrix portion
 */
struct thread_args {
    int *a;
    int *b;
    int *c;
    unsigned start;
    unsigned end;
};

/*
 * Function executed in each thread to compute its portion of the hadamard product
 * 'data': Pointer to struct thread_args instance
 * Declared here so this function can be referenced in hadamard_threads().
 * Complete the definition at the bottom of this file.
 * Returns NULL
 */
static void *worker(void *data);

PyObject *hadamard_threads(PyObject *self, PyObject *args) {
    /*
     * TODO Complete this function
     * Read arguments passed in by Python
     * Read the two input matrices from their files
     * Set up a result matrix on the heap
     * Launch 'n_workers' threads, each assigned a roughly equal portion of the
     * matrices to work on.
     * Wait for all threads to complete, then pass the result matrix to the
     * print_matrix() function.
     * Don't forget error handling and cleanup!
     */

    Py_RETURN_NONE;
}

void *worker(void *p) {
    /*
     * TODO Complete this function
     * Multiply each pair of elements and store the result, for the assigned
     * portion of the input matrices.
     */
}
