# SPDX-FileCopyrightText: 2026 John Kolb <jhkolb@umn.edu>
# SPDX-License-Identifier: GPL-3.0-or-later
from setuptools import setup, Extension

module_sources = [
    "hadamard_extension.c",
    "hadamard_utils.c",
    "hadamard_pipe.c",
    "hadamard_shmem.c",
    "hadamard_threads.c",
]

# Define the extension module
module = Extension("hadamard", sources=module_sources)

setup(
    name="hadamard",
    version="1.0",
    description="Compute hadmard product of two matrices in parallel",
    ext_modules=[module],
)
