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

PyObject *hadamard_shmem(PyObject *self, PyObject *args)
{
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
    // Check if args is a python list object
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
    // Set up shared memory segment for matrix A
    int *mat_a_data = read_matrix_from_file(mat_a_file, &a_rows, &a_cols, 1);
    if (mat_a_data == NULL)
    {
        return NULL;
    }
    // Calculate the total bytes of all matrix elements
    unsigned mat_elems = a_rows * a_cols;
    unsigned mat_size = mat_elems * sizeof(int);
    unsigned b_rows;
    unsigned b_cols;
    // Set up shared memory segment for matrix B
    int *mat_b_data = read_matrix_from_file(mat_b_file, &b_rows, &b_cols, 1);
    if (mat_b_data == NULL)
    {
        munmap(mat_a_data, mat_size);
        return NULL;
    }
    assert(a_rows == b_rows && a_cols == b_cols);
    // Set up shared memory segment for matrix C
    int *mat_c_data = mmap(NULL, mat_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mat_c_data == MAP_FAILED)
    {
        munmap(mat_a_data, mat_size);
        munmap(mat_b_data, mat_size);
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    // Give each child a floored chunk to work on
    int chunk_size = mat_elems / n_procs;
    // Fork specified amount of processes
    for (int i = 0; i < n_procs; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            munmap(mat_a_data, mat_size);
            munmap(mat_b_data, mat_size);
            munmap(mat_c_data, mat_size);
            PyErr_SetFromErrno(PyExc_OSError);
            return NULL;
        }
        // Have each child compute its specified workload
        else if (pid == 0)
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
            // Run work on specified portion of matrix C
            if (worker(mat_a_data, mat_b_data, mat_c_data, start, end, mat_size) == -1)
            {
                exit(1);
            }
            exit(0);
        }
    }
    // In parent process
    // Wait for children and check if they exited with 1
    int status;
    int work_failed = 0;
    for (int i = 0; i < n_procs; i++)
    {
        waitpid(-1, &status, 0);
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) == 1)
            {
                work_failed = 1;
            }
        }
    }
    // Print matrix C and clean up
    print_matrix(mat_c_data, a_rows, a_cols);
    munmap(mat_a_data, mat_size);
    munmap(mat_b_data, mat_size);
    munmap(mat_c_data, mat_size);
    // If a child exited with 1
    if (work_failed)
    {
        PyErr_SetString(PyExc_OSError, "Worker process failed");
    }
    // If all children exited with 0
    Py_RETURN_NONE;
}

// Called once per child process
static int worker(int *a, int *b, int *c, unsigned start, unsigned end, unsigned mapped_size)
{
    /*
     * TODO Complete this function
     * Multiply each pair of elements from the assigned portion of the input
     * matrices and store in the result matrix.
     * Don't forget error handling and cleanup!
     */
    // Check for null pointers
    if (a == NULL || b == NULL || c == NULL)
    {
        if (a != NULL)
        {
            munmap(a, mapped_size);
        }
        if (b != NULL)
        {
            munmap(b, mapped_size);
        }
        if (c != NULL)
        {
            munmap(c, mapped_size);
        }
        return -1;
    }
    // Check if start > end
    if (end == min(start, end))
    {
        munmap(a, mapped_size);
        munmap(b, mapped_size);
        munmap(c, mapped_size);
        return -1;
    }
    // Calculate specified portion of matrix C
    for (int i = start; i < end; i++)
    {
        c[i] = a[i] * b[i];
    }
    munmap(a, mapped_size);
    munmap(b, mapped_size);
    munmap(c, mapped_size);
    return 0;
}
