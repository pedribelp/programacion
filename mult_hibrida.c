/*
 * mult_hibrida.c — Multiplicacion de matrices C = A x B (enfoque hibrido)
 *
 * MPI:    distribuye bloques de filas de A entre procesos (Scatter/Gather).
 * OpenMP: paraleliza el calculo de cada bloque dentro del proceso.
 *
 * Compilacion:
 *   OMPI_CC=gcc-15 mpicc -fopenmp -O2 -o mult_hibrida mult_hibrida.c
 *
 * Ejecucion (4 procesos MPI, 2 hilos OpenMP cada uno):
 *   export OMP_NUM_THREADS=2
 *   mpirun -np 4 ./mult_hibrida
 *
 * N debe ser divisible entre el numero de procesos.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>

#define N 480   /* dimension de las matrices N x N */

/* Inicializa M con M[i][j] = (i+j) % 10 — valores deterministas para verificacion */
static void llenar_matriz(double *M, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            M[i * n + j] = (double) ((i + j) % 10);
}

int main(int argc, char *argv[]) {
    int rank, nproc;
    double *A = NULL, *B = NULL, *C = NULL;
    double *A_local, *C_local;
    int filas_local;
    double t_inicio, t_fin, t_total;

    /* 1) INICIALIZACION DEL ENTORNO MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);   /* identificador de este proceso */
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);  /* numero total de procesos */

    if (N % nproc != 0) {
        if (rank == 0)
            fprintf(stderr, "Error: N=%d debe ser divisible entre el numero de procesos (%d)\n", N, nproc);
        MPI_Finalize();
        return 1;
    }

    filas_local = N / nproc;          /* filas de A que recibe cada proceso */
    B = (double *) malloc(N * N * sizeof(double));
    A_local = (double *) malloc(filas_local * N * sizeof(double));
    C_local = (double *) malloc(filas_local * N * sizeof(double));

    /* Solo el proceso 0 crea las matrices completas A y C */
    if (rank == 0) {
        A = (double *) malloc(N * N * sizeof(double));
        C = (double *) malloc(N * N * sizeof(double));
        llenar_matriz(A, N);
        llenar_matriz(B, N);
    }

    /* La matriz B la necesitan TODOS los procesos completa, asi que se
     * difunde con MPI_Bcast desde el proceso 0 al resto. */
    MPI_Bcast(B, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);   /* sincroniza antes de medir el tiempo */
    t_inicio = MPI_Wtime();

    /* 2) DISTRIBUCION DE DATOS: cada proceso recibe un bloque de filas de A */
    MPI_Scatter(A, filas_local * N, MPI_DOUBLE,
                A_local, filas_local * N, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    /* 3) CALCULO PARALELO CON OPENMP DENTRO DE CADA PROCESO
     *
     * Cada proceso MPI calcula su bloque de "filas_local" filas de la
     * matriz resultado. La directiva "#pragma omp parallel for" reparte
     * esas filas entre los hilos OpenMP disponibles en este proceso.
     * "collapse(2)" funde los dos bucles externos (fila i, columna j) en
     * un solo espacio de iteracion para repartir mejor el trabajo entre
     * los hilos cuando filas_local es pequeno.
     */
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < filas_local; i++) {
        for (int j = 0; j < N; j++) {
            double suma = 0.0;
            for (int k = 0; k < N; k++) {
                suma += A_local[i * N + k] * B[k * N + j];
            }
            C_local[i * N + j] = suma;
        }
    }

    /* 4) RECOLECCION DE RESULTADOS: cada proceso envia su bloque calculado
     * y el proceso 0 los reune en la matriz C completa. */
    MPI_Gather(C_local, filas_local * N, MPI_DOUBLE,
               C, filas_local * N, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    /* 5) SINCRONIZACION antes de tomar el tiempo final: nos asegura que
     * TODOS los procesos hayan terminado, no solo el proceso 0. */
    MPI_Barrier(MPI_COMM_WORLD);
    t_fin = MPI_Wtime();
    t_total = t_fin - t_inicio;

    if (rank == 0) {
        /* Verificacion rapida: suma de todos los elementos de C */
        double suma_total = 0.0;
        for (int i = 0; i < N * N; i++) suma_total += C[i];

        printf("Procesos MPI: %d | Hilos OpenMP por proceso: %d | N=%d | "
               "SumaC=%.2f | Tiempo: %f segundos\n",
               nproc, omp_get_max_threads(), N, suma_total, t_total);

        free(A);
        free(C);
    }

    free(B);
    free(A_local);
    free(C_local);

    MPI_Finalize();
    return 0;
}
