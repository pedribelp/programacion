#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 200000000UL   /* Tamaño del arreglo: 200 millones de enteros */

int main(int argc, char *argv[]) {
    int num_threads = 1;
    if (argc > 1) {
        num_threads = atoi(argv[1]);
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }

    /* Reservamos memoria para el arreglo */
    int *arr = (int *) malloc(N * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "Error al reservar memoria\n");
        return 1;
    }

    /* Inicialización del arreglo (paralela también, para no influir
       en la medición posterior con el costo de inicialización serial) */
    #pragma omp parallel for
    for (unsigned long i = 0; i < N; i++) {
        arr[i] = (int)(i % 100);
    }

    long long suma = 0;
    double t_inicio, t_fin;

    t_inicio = omp_get_wtime();

    /*
     * Directiva principal de paralelización:
     *   #pragma omp parallel for reduction(+:suma)
     *
     * - "parallel for": reparte las iteraciones del bucle entre los
     *    hilos disponibles, cada hilo procesa un sub-rango del arreglo.
     * - "reduction(+:suma)": cada hilo mantiene una copia privada de
     *    "suma" donde acumula su parcial; al finalizar, OpenMP combina
     *    (suma) todas las copias parciales de forma segura para
     *    obtener el resultado total, evitando condiciones de carrera.
     */
    #pragma omp parallel for reduction(+:suma)
    for (unsigned long i = 0; i < N; i++) {
        suma += arr[i];
    }

    t_fin = omp_get_wtime();

    double tiempo = t_fin - t_inicio;

    printf("Hilos: %d | Suma: %lld | Tiempo: %f segundos\n",
           num_threads, suma, tiempo);

    free(arr);
    return 0;
}
