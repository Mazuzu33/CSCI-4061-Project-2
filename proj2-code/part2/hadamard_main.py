# SPDX-FileCopyrightText: 2026 John Kolb <jhkolb@umn.edu>
# SPDX-License-Identifier: GPL-3.0-or-later
import hadamard
import sys

if __name__ == "__main__":
    if len(sys.argv) < 5:
        print(f"Usage: {sys.argv[0]} <file_name_a> <file_name_b> <mode> <n_workers>")
        print('\tMode is one of "pipe", "shmem", or "threads"')
        sys.exit(1)

    mat_a_file = sys.argv[1]
    mat_b_file = sys.argv[2]
    mode = sys.argv[3]
    n_workers = int(sys.argv[4])
    assert n_workers > 0

    if mode == "pipe":
        hadamard.hadamard_pipe(mat_a_file, mat_b_file, n_workers)
    elif mode == "shmem":
        hadamard.hadamard_shmem(mat_a_file, mat_b_file, n_workers)
    elif mode == "threads":
        hadamard.hadamard_threads(mat_a_file, mat_b_file, n_workers)
    else:
        print(f'Unknown mode "{mode}"')
