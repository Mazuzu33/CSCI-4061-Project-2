#include <Python.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "hadamard.h"

/*
 * Represents one matrix element communicated over a pipe
 * 'idx': Element index (in flattened representation)
 * 'value': Element value
 */
struct element
{
    unsigned idx;
    int value;
};

/*
 * Function executed in each child process to compute its portion of the hadamard product
 * Declared here so it can be referenced in hadamard_pipe(). Complete the
 * definition at the bottom of this file.
 *
 * 'a': Pointer to elements for first matrix operand
 * 'b': Pointer to elements for second matrix operand
 * 'start': Index (in flattened representation) of first element of matrix portion
 * 'end': Index (in flattened representation) of last element of matrix portion
 * 'pipe_fd': Descriptor of write end of pipe to use to send results back to coordinator
 *
 * Returns 0 on success or -1 on failure
 */
static int worker(const int *a, const int *b, unsigned start, unsigned end, int pipe_fd);

PyObject *hadamard_pipe(PyObject *self, PyObject *args)
{
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

    /*
     * TODO Complete the implementation of this function
     * Create one pipe for all processes to share
     * Fork 'n_workers' child processes
     * Assign each child a roughly equal portion of matrix to work on
     * Set up a result matrix to store matrix elements sent by children
     * Collect results in the parent via the pipe
     * Don't forget cleanup and error handling!
     */

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        free(mat_a_data);
        free(mat_b_data);
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    int elements = a_cols * a_rows;
    int mat_size = elements * sizeof(int);
    int chunkElements = elements / n_procs;
    int remainder = elements % n_procs;

    for (int n = 0; n < n_procs; n++)
    {
        pid_t child_pid = fork();
        // free and close it
        if (child_pid == -1)
        {
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            free(mat_a_data);
            free(mat_b_data);
            // Wait for other children
            for (int j = 0; j < n; j++)
            {
                wait(NULL);
            }
            PyErr_SetFromErrno(PyExc_OSError);
            return NULL;
        }
        else if (child_pid == 0)
        {
            close(pipe_fd[0]);

            unsigned start;
            unsigned end;

            if (n < remainder)
            {
                start = n * chunkElements + n;
                end = (n + 1) * chunkElements + n + 1;
            }
            else
            {
                start = n * chunkElements + remainder;
                end = (n + 1) * chunkElements + remainder;
            }

            if (worker(mat_a_data, mat_b_data, start, end, pipe_fd[1]) == -1)
            {
                free(mat_a_data);
                free(mat_b_data);
                close(pipe_fd[1]);
                exit(1);
            }

            free(mat_a_data);
            free(mat_b_data);

            close(pipe_fd[1]);

            exit(0);
        }
    }

    free(mat_a_data);
    free(mat_b_data);
    close(pipe_fd[1]);

    // const char *mat_c_file;
    int *mat_c_data = malloc(mat_size);
    if (mat_c_data == NULL)
    {
        close(pipe_fd[0]);
        for (int i = 0; i < n_procs; i++)
        {
            wait(NULL);
        }
        return PyErr_NoMemory();
    }

    for (unsigned n = 0; n < elements; n++)
    {
        struct element e;
        int bytes_read = read(pipe_fd[0], &e, sizeof(struct element));
        // Error reading bytes
        if (bytes_read == -1)
        {
            // Handle error reading
            close(pipe_fd[0]);
            free(mat_c_data);
            for (int i = 0; i < n_procs; i++)
            {
                wait(NULL);
            }
            PyErr_SetFromErrno(PyExc_OSError);
            return NULL;
        }
        // EOF reached early or read of incorrect size
        else if (bytes_read != sizeof(struct element))
        {
            break;
        }
        mat_c_data[e.idx] = e.value;
    }

    // close reading
    close(pipe_fd[0]);

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
    // If a child exited with 1
    if (work_failed)
    {
        free(mat_c_data);
        PyErr_SetString(PyExc_OSError, "Worker process failed");
        return NULL;
    }
    // If all children exited with 0
    print_matrix(mat_c_data, a_rows, a_cols);
    free(mat_c_data);
    Py_RETURN_NONE;
}

// Called once by each child process
static int worker(const int *a, const int *b, unsigned start, unsigned end, int pipe_fd)
{
    /*
     * TODO Complete this function's implementation
     * Open the two matrix data files
     * Read elements from the matrix files corresponding to this child's assigned
     * portion of the matrix
     * Multiply the two elements, then send result via pipe
     * Don't forget proper error handling and cleanup!
     */
    // Check for null pointers
    if (a == NULL || b == NULL)
    {
        return -1;
    }
    // Check if start > end
    if (end == min(start, end))
    {
        return -1;
    }
    for (unsigned i = start; i < end; i++)
    {
        struct element e = {i, a[i] * b[i]};
        if (write(pipe_fd, &e, sizeof(struct element)) != sizeof(struct element))
        {
            return -1;
        }
    }
    return 0;
}
