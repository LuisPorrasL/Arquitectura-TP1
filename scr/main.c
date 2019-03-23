#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv); // Inicializacion del ambiente para MPI.
    fprintf(stdout, "Hello world!\n");
    MPI_Finalize(); // Se termina el ambiente MPI
    return 0;
}

