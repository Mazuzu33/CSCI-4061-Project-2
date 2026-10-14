// SPDX-FileCopyrightText: 2026 John Kolb <jhkolb@umn.edu>
// SPDX-License-Identifier: GPL-3.0-or-later
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>

#include "hadamard.h"

int *read_matrix_from_file(const char *file_name, unsigned *n_rows, unsigned *n_columns,
                           int sh_mem) {
    FILE *f = fopen(file_name, "r");
    if (f == NULL) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    if (fread(n_rows, sizeof(*n_rows), 1, f) != 1 ||
        fread(n_columns, sizeof(*n_columns), 1, f) != 1) {
        PyErr_SetString(PyExc_OSError, "Unable to read dimensions from matrix file\n");
        fclose(f);
        return NULL;
    }

    unsigned total_elems = *n_rows * *n_columns;
    unsigned mat_size = total_elems * sizeof(int);
    int *matrix_data;
    if (sh_mem) {
        matrix_data =
            mmap(NULL, mat_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (matrix_data == MAP_FAILED) {
            PyErr_SetFromErrno(PyExc_OSError);
            fclose(f);
            return NULL;
        }
    } else {
        matrix_data = malloc(mat_size);
        if (matrix_data == NULL) {
            fclose(f);
            return PyErr_NoMemory();
        }
    }

    if (fread(matrix_data, sizeof(int), total_elems, f) < total_elems) {
        PyErr_SetString(PyExc_OSError, "Unable to read from matrix data file\n");
        if (sh_mem) {
            munmap(matrix_data, mat_size);
        } else {
            free(matrix_data);
        }
        fclose(f);
        return NULL;
    }

    if (fclose(f) != 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        if (sh_mem) {
            munmap(matrix_data, mat_size);
        } else {
            free(matrix_data);
        }
        return NULL;
    }

    return matrix_data;
}

void print_matrix(const int *mat, unsigned n_rows, unsigned n_cols) {
    for (unsigned i = 0; i < n_rows; i++) {
        for (unsigned j = 0; j < n_cols; j++) {
            printf("%d", mat[i * n_cols + j]);
            if (j < n_cols - 1) {
                printf(", ");
            }
        }
        printf("\n");
    }
}

unsigned min(unsigned a, unsigned b) {
    if (a < b) {
        return a;
    }
    return b;
}
