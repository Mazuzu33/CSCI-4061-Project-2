// SPDX-FileCopyrightText: 2026 John Kolb <jhkolb@umn.edu>
// SPDX-License-Identifier: GPL-3.0-or-later
#include <Python.h>
#include <stdio.h>

#include "hadamard.h"

static PyMethodDef hadamard_methods[] = {
    {
        .ml_name = "hadamard_pipe",
        .ml_meth = hadamard_pipe,
        .ml_flags = METH_VARARGS,
        .ml_doc = "Compute Hadamard Product with multiple processes communicating via a pipe",
    },
    {
        .ml_name = "hadamard_shmem",
        .ml_meth = hadamard_shmem,
        .ml_flags = METH_VARARGS,
        .ml_doc = "Compute Hadamard Product with multiple processes communicating via shared memory",
    },
    {
        .ml_name = "hadamard_threads",
        .ml_meth = hadamard_threads,
        .ml_flags = METH_VARARGS,
        .ml_doc = "Compute Hadamard Product with multiple threads",
    },
    {
        .ml_name = NULL,
        .ml_meth = NULL,
        .ml_flags = 0,
        .ml_doc = NULL,
    }};

static struct PyModuleDef hadamard_module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "hadamard",
    .m_doc = "Compute the hadamard product of two matrices in parallel",
    .m_size = -1,
    .m_methods = hadamard_methods,
};

PyMODINIT_FUNC PyInit_hadamard(void) {
    return PyModule_Create(&hadamard_module);
}
