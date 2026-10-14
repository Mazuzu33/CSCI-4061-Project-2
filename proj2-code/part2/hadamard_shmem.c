#define _GNU_SOURCE

#include <Python.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "hadamard.h"

/*
 * Function executed in each child process to compute its portion of the hadamard product
 * Declared here so it can be referenced in hadamard_shmem. Fill in the definition at
 * the end of this file.
 *
 * 'a': Pointer to shared memory segment storing first matrix operand
 * 'b': Pointer to shared memory segment storing second matrix operand
 * 'c': Pointer to shared memory segment storing result matrix
 * 'start': Index (in flattened representation) of first element of matrix portion
 * 'end': Index (in flattened representation) of last element of matrix portion
 * 'mapped_size': Size (in bytes) of each shared memory segment
 *
 * Returns 0 on success or -1 on failure
 */
static int worker(int *a, int *b, int *c, unsigned start, unsigned end, unsigned mapped_size);

PyObject *hadamard_shmem(PyObject *self, PyObject *args) {
    /*
     * TODO Implement this function
     * Read in arguments passed from Python
     * Read the two operand matrices from their files into shared memory
     * Set up shared memory segment to hold result matrix
     * Fork 'n_workers' children, giving each a roughly equal portion of the
     * input matrices to work on
     * Wait for all children to complete their work back in the parent
     * Then pass the result matrix to print_matrix()
     *
     * Don't forget error handling and cleanup!
     */

    Py_RETURN_NONE;
}

// Called once per child process
static int worker(int *a, int *b, int *c, unsigned start, unsigned end, unsigned mapped_size) {
    /*
     * TODO Complete this function
     * Multiply each pair of elements from the assigned portion of the input
     * matrices and store in the result matrix.
     * Don't forget error handling and cleanup!
     */

    return 0;
}
