#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <oneapi/tbb.h>

// PASO 1: Eleva al cuadrado cada elemento del arreglo en paralelo
void cuadradoParalelo(std::vector<double>& datos) {
    int n = static_cast<int>(datos.size());

    tbb::parallel_for(
        tbb::blocked_range<int>(0, n),
        [&datos](const tbb::blocked_range<int>& r) {
            for (int i = r.begin(); i < r.end(); ++i)
                datos[i] = datos[i] * datos[i];
        }
    );
}

// PASO 2: Suma todos los elementos del arreglo en paralelo
double sumaParalelaTBB(const std::vector<double>& arreglo) {
    int n = static_cast<int>(arreglo.size());

    return tbb::parallel_reduce(
        tbb::blocked_range<int>(0, n),  // Rango a procesar
        0.0,                             // Valor inicial
        [&arreglo](const tbb::blocked_range<int>& r, double acc) -> double {
            for (int i = r.begin(); i < r.end(); ++i)
                acc += arreglo[i];
            return acc;
        },
        [](double a, double b) -> double {
            return a + b;               // Une los resultados de cada hilo
        }
    );
}

// PASO 3: Dos tareas distintas corriendo al mismo tiempo
void tareasIndependientes() {
    std::cout << "\n=== TAREAS INDEPENDIENTES ===" << std::endl;

    double resultadoA = 0.0;
    double resultadoB = 0.0;

    tbb::parallel_invoke(
        [&resultadoA]() {
            for (int i = 1; i <= 1000000; ++i)
                resultadoA += 1.0 / i;
            std::cout << "  Tarea A completada: serie armonica = "
                      << resultadoA << std::endl;
        },
        [&resultadoB]() {
            for (int i = 1; i <= 1000000; ++i)
                resultadoB += static_cast<double>(i) * i;
            std::cout << "  Tarea B completada: suma de cuadrados lista"
                      << std::endl;
        }
    );
    std::cout << "Ambas tareas completadas" << std::endl;
}

// PASO 4: Grupo de tareas creadas dinamicamente
void gruposDeTareas() {
    std::cout << "\n=== GRUPOS DE TAREAS ===" << std::endl;

    tbb::task_group grupo;

    for (int id = 0; id < 4; ++id) {
        grupo.run([id]() {
            double suma = 0.0;
            for (int j = id * 100; j < (id + 1) * 100; ++j)
                suma += j;
            std::cout << "  Tarea " << id
                      << " | Suma parcial: " << suma << std::endl;
        });
    }

    grupo.wait(); // Espera a que todas las tareas terminen
    std::cout << "Todas las tareas finalizaron" << std::endl;
}

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "  DEMO TBB - Threading Building Blocks     " << std::endl;
    std::cout << "============================================" << std::endl;

    const int TAMANIO = 10000000;
    std::vector<double> datos(TAMANIO);
    std::iota(datos.begin(), datos.end(), 1.0);

    std::cout << "\n=== PASO 1: parallel_for ===" << std::endl;
    std::cout << "datos[0] antes: " << datos[0] << std::endl;

    auto inicio = std::chrono::high_resolution_clock::now();
    cuadradoParalelo(datos);
    auto fin = std::chrono::high_resolution_clock::now();
    double tiempoPFor = std::chrono::duration<double>(fin - inicio).count();

    std::cout << "datos[0] despues (1^2): " << datos[0] << std::endl;
    std::cout << "datos[2] despues (3^2): " << datos[2] << std::endl;
    std::cout << "Tiempo: " << tiempoPFor << " s" << std::endl;

    std::cout << "\n=== PASO 2: parallel_reduce ===" << std::endl;

    // Version secuencial (lenta)
    inicio = std::chrono::high_resolution_clock::now();
    double sumaSeq = 0.0;
    for (const auto& x : datos) sumaSeq += x;
    fin = std::chrono::high_resolution_clock::now();
    double tiempoSeq = std::chrono::duration<double>(fin - inicio).count();

    // Version paralela con TBB (rapida)
    inicio = std::chrono::high_resolution_clock::now();
    double sumaTBB = sumaParalelaTBB(datos);
    fin = std::chrono::high_resolution_clock::now();
    double tiempoTBB = std::chrono::duration<double>(fin - inicio).count();

    std::cout << "Secuencial: " << sumaSeq
              << " | Tiempo: " << tiempoSeq << " s" << std::endl;
    std::cout << "Con TBB:    " << sumaTBB
              << " | Tiempo: " << tiempoTBB << " s" << std::endl;
    std::cout << "Aceleracion: " << tiempoSeq / tiempoTBB << "x" << std::endl;

    tareasIndependientes();
    gruposDeTareas();

    std::cout << "\n============================================" << std::endl;
    std::cout << "  Demo TBB completada exitosamente          " << std::endl;
    std::cout << "============================================" << std::endl;

    return 0;
}
