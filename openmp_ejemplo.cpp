#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

// PASO 1: Suma paralela - cada hilo suma su parte, al final se combinan
double sumaParalela(const std::vector<double>& arreglo) {
    double suma = 0.0;
    int n = static_cast<int>(arreglo.size());

    #pragma omp parallel for reduction(+:suma) schedule(static)
    for (int i = 0; i < n; i++) {
        suma += arreglo[i];
    }
    return suma;
}

// PASO 2: Multiplicacion paralela - cada hilo trabaja en posiciones distintas
std::vector<double> multiplicacionParalela(
    const std::vector<double>& a,
    const std::vector<double>& b)
{
    int n = static_cast<int>(a.size());
    std::vector<double> resultado(n);

    #pragma omp parallel for schedule(dynamic, 100)
    for (int i = 0; i < n; i++) {
        resultado[i] = a[i] * b[i];
    }
    return resultado;
}

// PASO 3: Muestra cuantos hilos tiene disponibles la computadora
void mostrarInfoHilos() {
    std::cout << "\n=== INFORMACION DE HILOS ===" << std::endl;
    std::cout << "Hilos disponibles: " << omp_get_max_threads() << std::endl;

    #pragma omp parallel
    {
        #pragma omp single
        {
            std::cout << "Hilos activos: " << omp_get_num_threads() << std::endl;
        }
    }
}

// PASO 4: Dos tareas completamente distintas corriendo al mismo tiempo
void seccionesParalelas() {
    std::cout << "\n=== SECCIONES PARALELAS ===" << std::endl;

    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            std::cout << "[Hilo " << omp_get_thread_num()
                      << "] Tarea A: procesamiento de datos" << std::endl;
        }

        #pragma omp section
        {
            std::cout << "[Hilo " << omp_get_thread_num()
                      << "] Tarea B: analisis estadistico" << std::endl;
        }
    }
    std::cout << "Ambas tareas completadas" << std::endl;
}

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "  DEMO OpenMP - Programacion Paralela       " << std::endl;
    std::cout << "============================================" << std::endl;

    mostrarInfoHilos();

    const int TAMANIO = 10'000'000;
    std::vector<double> vecA(TAMANIO, 1.5);
    std::vector<double> vecB(TAMANIO, 2.0);

    std::cout << "\n=== SUMA PARALELA ===" << std::endl;
    std::cout << "Arreglo: " << TAMANIO << " elementos" << std::endl;

    // Version secuencial (lenta)
    auto inicio = std::chrono::high_resolution_clock::now();
    double sumaSecuencial = 0.0;
    for (int i = 0; i < TAMANIO; i++) {
        sumaSecuencial += vecA[i];
    }
    auto fin = std::chrono::high_resolution_clock::now();
    double tiempoSecuencial = std::chrono::duration<double>(fin - inicio).count();
    std::cout << "Secuencial: " << sumaSecuencial
              << " | Tiempo: " << tiempoSecuencial << " s" << std::endl;

    // Version paralela con OpenMP (rapida)
    inicio = std::chrono::high_resolution_clock::now();
    double sumaOMP = sumaParalela(vecA);
    fin = std::chrono::high_resolution_clock::now();
    double tiempoParalelo = std::chrono::duration<double>(fin - inicio).count();
    std::cout << "Con OpenMP: " << sumaOMP
              << " | Tiempo: " << tiempoParalelo << " s" << std::endl;
    std::cout << "Aceleracion: " << tiempoSecuencial / tiempoParalelo << "x" << std::endl;

    std::cout << "\n=== MULTIPLICACION PARALELA ===" << std::endl;
    inicio = std::chrono::high_resolution_clock::now();
    std::vector<double> resultado = multiplicacionParalela(vecA, vecB);
    fin = std::chrono::high_resolution_clock::now();
    double tiempoMult = std::chrono::duration<double>(fin - inicio).count();
    std::cout << "Resultado[0] = " << resultado[0] << " (esperado: 3.0)" << std::endl;
    std::cout << "Tiempo: " << tiempoMult << " s" << std::endl;

    seccionesParalelas();

    std::cout << "\n============================================" << std::endl;
    std::cout << "  Demo OpenMP completada exitosamente       " << std::endl;
    std::cout << "============================================" << std::endl;

    return 0;
}
