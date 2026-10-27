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
    pipe(pipe_fd);

    int elements = a_cols * a_rows;
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
            perror("pipe failed to open");
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

            worker(mat_a_data, mat_b_data, start, end, pipe_fd[1]);

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
    int mat_c_data[elements];
    memset(mat_c_data, 0, sizeof(mat_c_data));

    for (unsigned n = 0; n < elements; n++)
    {
        struct element e;
        if (read(pipe_fd[0], &e, sizeof(struct element)) != sizeof(struct element))
        {
            // Handle read error or unexpected end of pipe.
            break;
        }

        mat_c_data[e.idx] = e.value;
    }

    close(pipe_fd[0]);

    // close writing
    for (int n = 0; n < chunkElements; n++)
    {
        wait(NULL);
    }

    print_matrix(mat_c_data, a_rows, a_cols);

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

    for (unsigned i = start; i < end; i++)
    {
        struct element e = {i, a[i] * b[i]};
        write(pipe_fd, &e, sizeof(struct element));
    }

    return 0;
}
