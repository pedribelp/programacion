#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {

    int rank;
    int size;
    int valor = 0;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            fprintf(stderr, "Error: Este programa requiere al menos 2 procesos.\n");
            fprintf(stderr, "Ejecute con: mpirun -np 2 ./mpi_p2p\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        valor = 100;
        printf("[Proceso %d] Enviando valor: %d al proceso 1\n", rank, valor);
        MPI_Send(&valor, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("[Proceso %d] Mensaje enviado exitosamente.\n", rank);

    } else if (rank == 1) {
        MPI_Recv(&valor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        printf("[Proceso %d] Valor recibido: %d (enviado por el proceso %d)\n",
               rank, valor, status.MPI_SOURCE);
    }

    MPI_Finalize();

    return 0;
}
