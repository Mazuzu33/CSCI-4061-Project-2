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
    // don't read from pipe
    if (in_idx != -1)
    {
        if (dup2(pipes[in_idx], STDIN_FILENO) == -1)
        {
            close(pipes[in_idx]);
            if (out_idx != -1)
            {
                close(pipes[out_idx]);
            }
            perror("dup2");
            return -1;
        }
        close(pipes[in_idx]);
    }
    // don't write from pipe
    if (out_idx != -1)
    {
        if (dup2(pipes[out_idx], STDOUT_FILENO) == -1)
        {
            close(pipes[out_idx]);
            perror("dup2");
            return -1;
        }
        close(pipes[out_idx]);
    }
    if (run_command(tokens) == -1)
    {
        fprintf(stderr, "Failed to run command\n");
        return -1;
    }
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
        if (pipe(&pipe_fds[2 * i]) == -1)
        {
            perror("pipe");
            // Close all pipe descripters that have been created so far
            for (int j = 0; j < 2 * i; j++)
            {
                close(pipe_fds[j]);
            }
            return -1;
        }
    }
    // Fork a child process for every pipe with one additional
    for (int i = numPipes; i >= 0; i--)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            // Close all pipe descriptors
            for (int j = 0; j < 2 * numPipes; j++)
            {
                close(pipe_fds[j]);
            }
            // Wait for all children
            for (int j = 0; j < i; j++)
            {
                wait(NULL);
            }
            return -1;
        }
        // For each child process, run its specified command
        if (pid == 0)
        {
            // Close all pipe fds except for the ones it will read from or write to
            int readEnd = i * 2 - 2;
            int writeEnd = i * 2 + 1;
            // Set readEnd to -1 if this is the first command
            if (readEnd == -2)
            {
                readEnd = -1;
            }
            // Set writeEnd to -1 if this is the last command
            if (writeEnd == numPipes * 2 + 1)
            {
                writeEnd = -1;
            }
            for (int j = 0; j < 2 * numPipes; j++)
            {
                if (j != readEnd && j != writeEnd)
                {
                    close(pipe_fds[j]);
                }
            }
            // Get the program name and arguments for the specified process
            // Make a copy of tokens
            strvec_t copyTokens;
            if (strvec_slice(tokens, &copyTokens, 0, tokens->length) == -1)
            {
                fprintf(stderr, "Tokens slicing failed\n");
                // Close appropriate pipe descriptors
                if (readEnd != -1)
                {
                    close(pipe_fds[readEnd]);
                }
                if (writeEnd != -1)
                {
                    close(pipe_fds[writeEnd]);
                }
                // Free tokens
                strvec_clear(tokens);
                exit(1);
            }
            // Get the index of the pipe that comes after the command, which is set to the length of tokens if it doesn't exist
            int endPipeIndex = tokens->length;
            for (int j = numPipes; j > i; j--)
            {
                endPipeIndex = strvec_find_last(&copyTokens, "|");
                if (strvec_slice(tokens, &copyTokens, 0, endPipeIndex) == -1)
                {
                    fprintf(stderr, "Tokens slicing failed\n");
                    // Close appropriate pipe descriptors
                    if (readEnd != -1)
                    {
                        close(pipe_fds[readEnd]);
                    }
                    if (writeEnd != -1)
                    {
                        close(pipe_fds[writeEnd]);
                    }
                    // Free copyTokens and tokens
                    strvec_clear(&copyTokens);
                    strvec_clear(tokens);
                    exit(1);
                }
            }
            // Get the index of the pipe that comes before the command, which is set to 0 if it doesn't exist
            int startPipeIndex = strvec_find_last(&copyTokens, "|") + 1;
            // Free copyTokens
            strvec_clear(&copyTokens);
            // Get the tokens for the speicified command
            strvec_t command_tokens;
            if (strvec_slice(tokens, &command_tokens, startPipeIndex, endPipeIndex) == -1)
            {
                fprintf(stderr, "Tokens slicing failed\n");
                // Close appropriate pipe descriptors
                if (readEnd != -1)
                {
                    close(pipe_fds[readEnd]);
                }
                if (writeEnd != -1)
                {
                    close(pipe_fds[writeEnd]);
                }
                strvec_clear(tokens);
                exit(1);
            }
            // run_piped_command will close its specified pipe_fds for us
            if (run_piped_command(&command_tokens, pipe_fds, numPipes * 2, readEnd, writeEnd) == -1)
            {
                fprintf(stderr, "Couldn't run a pipelined command\n");
                // Free commandTokens and tokens
                strvec_clear(&command_tokens);
                strvec_clear(tokens);
                exit(1);
            }
            // Free commandTokens and tokens
            strvec_clear(&command_tokens);
            strvec_clear(tokens);
            exit(0);
        }
    }
    // In parent process, close all pipe fds
    for (int j = 0; j < 2 * numPipes; j++)
    {
        close(pipe_fds[j]);
    }
    // In parent process, wait for children and check if they exited with 1
    int status;
    int ret_val = 0;
    for (int j = 0; j <= numPipes; j++)
    {
        waitpid(-1, &status, 0);
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) == 1)
            {
                ret_val = -1;
            }
        }
    }
    return ret_val;
}
