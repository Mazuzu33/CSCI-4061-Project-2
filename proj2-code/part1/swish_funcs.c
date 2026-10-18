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
    // Get the number of pipes and set them up
    int numPipes = strvec_num_occurrences(tokens, "|");
    int pipe_fds[numPipes * 2];
    for (int i = 0; i < numPipes; i++)
    {
        if (pipe(pipe_fds[2 * i]) == -1)
        {
            perror("pipe");
            // Close all pipe descripters that have been created so far
            for (int j = 0; j < 2 * i; j++)
            {
                close(pipe_fds[j]);
            }
        }
    }
    // Fork a child process for every pipe with one additional
    for (int i = numPipes; i >= 0; i--)
    {
        // Check if fork returns an error
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            // Close all pipe descriptors
            for (int j = 0; j < 2 * numPipes; j++)
            {
                close(pipe_fds[j]);
            }
        }
        // For each child process, run its specified command
        if (pid == 0)
        {
            // Close all pipe fds except for the ones it will read from or write to
            int readEnd = i * 2 - 1;
            int writeEnd = i * 2;
            for (int j = 0; j < 2 * numPipes; j++)
            {
                if (j != readEnd && j != writeEnd)
                {
                    close(pipe_fds[j]);
                }
            }
            // Set command to read from previous pipe and write to next pipe
            int in_idx = i - 1;
            int out_idx = i + 1;
            // Do not read from previous pipe for the first command. Do not write to next pipe for the last command.
            if (i == 0)
            {
                in_idx = -1;
            }
            else if (i == numPipes)
            {
                out_idx = -1;
            }
            // Get the program name and arguments for the specified process
            // Make a copy of tokens
            strvec_t *copyTokens;
            if (strvec_slice(tokens, copyTokens, 0, tokens->length) == -1)
            {
                fprintf(stderr, "Tokens slicing failed\n");
                // Close appropriate pipe descriptors
                close(pipe_fds[readEnd]);
                close(pipe_fds[writeEnd]);
            }
            // Get the index of the pipe that comes after the command, which is set to -1 if it doesn't exist
            int endPipeIndex = -1;
            for (int j = numPipes; j > i; j--)
            {
                endPipeIndex = strvec_find_last(copyTokens, "|");
                if (strvec_slice(copyTokens, copyTokens, 0, endPipeIndex) == -1)
                {
                    fprintf(stderr, "Tokens slicing failed\n");
                    // Close appropriate pipe descriptors
                    close(pipe_fds[readEnd]);
                    close(pipe_fds[writeEnd]);
                }
            }
            // Get the index of the pipe that comes before the command, which is set to -1 if it doesn't exist
            int startPipeIndex = strvec_find_last(copyTokens, "|");
            // Get the tokens for the speicified command
            strvec_t *command_tokens;
            strvec_slice(tokens, command_tokens, startPipeIndex, endPipeIndex);
            if (run_pipelined_command(command_tokens, pipe_fds, numPipes * 2, in_idx, out_idx) == -1)
            {
                fprintf(stderr, "Couldn't run a pipelined command\n");
                // Close appropriate pipe descriptors
                close(pipe_fds[readEnd]);
                close(pipe_fds[writeEnd]);
            }
        }
    }
    // In parent process, close all pipe fds
    for (int j = 0; j < 2 * numPipes; j++)
    {
        close(pipe_fds[j]);
    }
    return 0;
}
