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
struct thread_args
{
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

PyObject *hadamard_threads(PyObject *self, PyObject *args)
{
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
    const char *mat_a_file;
    const char *mat_b_file;
    unsigned n_procs;
    if (!PyArg_ParseTuple(args, "ssI", &mat_a_file, &mat_b_file, &n_procs))
    {
        // ParseTuple sets up a Python exception on our behalf
        return NULL;
    }

    unsigned a_rows;
    unsigned a_cols;
    int *mat_a_data = read_matrix_from_file(mat_a_file, &a_rows, &a_cols, 0);
    if (mat_a_data == NULL)
    {
        return NULL;
    }
    unsigned b_rows;
    unsigned b_cols;
    int *mat_b_data = read_matrix_from_file(mat_b_file, &b_rows, &b_cols, 0);
    if (mat_b_data == NULL)
    {
        free(mat_a_data);
        return NULL;
    }
    assert(a_rows == b_rows && a_cols == b_cols);
    // Calculate matrix elements and size
    unsigned mat_elems = a_rows * a_cols;
    unsigned mat_size = mat_elems * sizeof(int);
    int *mat_c_data = malloc(mat_size);
    if (mat_c_data == NULL)
    {
        free(mat_a_data);
        free(mat_b_data);
        return PyErr_NoMemory();
    }
    // Give each child a floored chunk to work on
    int chunk_size = mat_elems / n_procs;
    // Make array to hold thread ids
    pthread_t tids[n_procs];
    // Make array to hold thread arguments
    struct thread_args targs[n_procs];
    // Fork specified amount of processes
    int error_code;
    for (int i = 0; i < n_procs; i++)
    {
        // Calculate start and end. For the last process, make sure to take in the leftover elements.
        int start = i * chunk_size;
        int end;
        if (i == (n_procs - 1))
        {
            end = mat_elems;
        }
        else
        {
            end = (i + 1) * chunk_size;
        }
        // Set up arguments to be passed in to thread
        targs[i].a = mat_a_data;
        targs[i].b = mat_b_data;
        targs[i].c = mat_c_data;
        targs[i].start = start;
        targs[i].end = end;
        // Run work on specified portion of matrix C
        if ((error_code = pthread_create(&tids[i], NULL, worker, &targs[i])) != 0)
        {
            // Join threads already created
            for (int j = 0; j < i; j++)
            {
                pthread_join(tids[j], NULL);
            }
            free(mat_a_data);
            free(mat_b_data);
            free(mat_c_data);
            PyErr_Format(PyExc_OSError, "Error trying to create thread: %s\n", strerror(error_code));
            return NULL;
        }
    }
    // In parent process
    // Wait for threads
    for (int i = 0; i < n_procs; i++)
    {
        if ((error_code = pthread_join(tids[i], NULL)) != 0)
        {
            // Join other threads and return an error message
            for (int j = i + 1; j < n_procs; j++)
            {
                pthread_join(tids[j], NULL);
            }
            free(mat_c_data);
            PyErr_Format(PyExc_OSError, "Error joining thread: %s\n", strerror(error_code));
            return NULL;
        }
    }
    free(mat_a_data);
    free(mat_b_data);

    // Print matrix
    print_matrix(mat_c_data, a_rows, a_cols);
    free(mat_c_data);
    Py_RETURN_NONE;
}

void *worker(void *p)
{
    /*
     * TODO Complete this function
     * Multiply each pair of elements and store the result, for the assigned
     * portion of the input matrices.
     */
    struct thread_args *args = (struct thread_args *)p;
    int *a = args->a;
    int *b = args->b;
    int *c = args->c;
    unsigned start = args->start;
    unsigned end = args->end;
    for (int i = start; i < end; i++)
    {
        c[i] = a[i] * b[i];
    }
    return NULL;
}
