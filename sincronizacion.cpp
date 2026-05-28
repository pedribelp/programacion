#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

const int NUM_HILOS      = 5;
const int ITERACIONES    = 1000;
const int VALOR_ESPERADO = NUM_HILOS * ITERACIONES;
const char* SEM_NOMBRE   = "/sem_sincronizacion";

long long contador_compartido = 0;
sem_t* semaforo = nullptr;

void* incrementar(void* arg) {
    int id = *((int*)arg);

    for (int i = 0; i < ITERACIONES; ++i) {
        sem_wait(semaforo);
        contador_compartido++;
        sem_post(semaforo);
    }

    std::cout << "[Hilo " << id << "] Finalizado.\n";
    return nullptr;
}

long long contador_sin_sync = 0;

void* incrementar_sin_sync(void* arg) {
    for (int i = 0; i < ITERACIONES; ++i) {
        long long tmp = contador_sin_sync;
        sched_yield();
        contador_sin_sync = tmp + 1;
    }
    return nullptr;
}

int main() {
    pthread_t hilos[NUM_HILOS];
    pthread_t hilos_sin_sync[NUM_HILOS];
    int ids[NUM_HILOS];

    std::cout << "=================================================\n";
    std::cout << "  Sincronizacion con Semaforos POSIX\n";
    std::cout << "=================================================\n";
    std::cout << "Hilos: " << NUM_HILOS
              << "  |  Iteraciones/hilo: " << ITERACIONES
              << "  |  Esperado: " << VALOR_ESPERADO << "\n\n";

    sem_unlink(SEM_NOMBRE);
    semaforo = sem_open(SEM_NOMBRE, O_CREAT | O_EXCL, 0644, 1);
    if (semaforo == SEM_FAILED) {
        std::cerr << "Error al crear el semaforo\n";
        return 1;
    }

    std::cout << "--- CON sincronizacion (semaforo) ---\n";
    for (int i = 0; i < NUM_HILOS; ++i) {
        ids[i] = i + 1;
        pthread_create(&hilos[i], nullptr, incrementar, &ids[i]);
    }
    for (int i = 0; i < NUM_HILOS; ++i)
        pthread_join(hilos[i], nullptr);

    std::cout << "\nValor final CON sync  : " << contador_compartido << "\n";
    std::cout << "Valor esperado        : " << VALOR_ESPERADO << "\n";
    std::cout << "Resultado correcto    : "
              << (contador_compartido == VALOR_ESPERADO ? "SI" : "NO") << "\n\n";

    sem_close(semaforo);
    sem_unlink(SEM_NOMBRE);

    contador_sin_sync = 0;

    std::cout << "--- SIN sincronizacion (condicion de carrera) ---\n";
    for (int i = 0; i < NUM_HILOS; ++i)
        pthread_create(&hilos_sin_sync[i], nullptr, incrementar_sin_sync, &ids[i]);
    for (int i = 0; i < NUM_HILOS; ++i)
        pthread_join(hilos_sin_sync[i], nullptr);

    std::cout << "Valor final SIN sync  : " << contador_sin_sync << "\n";
    std::cout << "Valor esperado        : " << VALOR_ESPERADO << "\n";
    std::cout << "Perdida de incrementos: " << (VALOR_ESPERADO - contador_sin_sync) << "\n";
    std::cout << "=================================================\n";

    return 0;
}
