#include "swish_funcs.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "string_vector.h"

#define MAX_ARGS 10

/*
 * Helper function to run a single command within a pipeline. You should make
 * use of the provided 'run_command' function here.
 * tokens: String vector containing the tokens representing the command to be
 * executed, possible redirection, and the command's arguments.
 * pipes: An array of pipe file descriptors.
 * n_pipe_fds: Length of the 'pipes' array
 * in_idx: Index of the file descriptor in the array from which the program
 *         should read its input, or -1 if input should not be read from a pipe.
 * out_idx: Index of the file descriptor in the array to which the program
 *          should write its output, or -1 if output should not be written to
 *          a pipe.
 * Returns 0 on success or -1 on error.
 */
int run_piped_command(strvec_t *tokens, int *pipes, int n_pipe_fds, int in_idx, int out_idx)
{
    // TODO Complete this function's implementation

    int pid = fork();

    if (pid == -1)
    {
        perror("failed to fork");
        return -1;
    }
    else if (pid == 0)
    {
        // don't read from pipe
        if (in_idx == -1)
        {
            dup2(pipes[out_idx], STDOUT_FILENO);
        }
        // don't write from pipe
        if (out_idx == -1)
        {
            dup2(pipes[in_idx], STDIN_FILENO);
        }

        int tokens_length = tokens->length;
        char args[tokens_length + 1];

        // turn tokens into args array and put NULL at the end
        for (int arg = 0; args < tokens_length; arg++)
        {
            args[arg] = strvec_get(tokens, arg);
        }
        args[tokens_length] = NULL;

        if (run_command(tokens) == -1)
        {
            perror("failed to run command");
            return -1;
        }

        return 0;
    }

    wait(NULL);

    return 0;
}

int run_pipelined_commands(strvec_t *tokens)
{
    // TODO Complete this function's implementation
    return 0;
}
