"""
Problema: Calcular la suma de cuadrados de números en rangos grandes
"""

import threading
import multiprocessing
import time
import math

# FUNCIÓN BASE: suma de cuadrados en un rango [inicio, fin)

def suma_cuadrados(inicio: int, fin: int) -> int:
    """Calcula la suma de cuadrados: inicio² + (inicio+1)² + ... + (fin-1)²"""
    return sum(i * i for i in range(inicio, fin))

# 1. ENFOQUE SECUENCIAL
def secuencial(n: int) -> int:
    """Calcula la suma de cuadrados de 0 a n de forma secuencial."""
    return suma_cuadrados(0, n)

# 2. ENFOQUE CON THREADING (Concurrencia)
def con_threading(n: int, num_hilos: int = 4) -> int:
    """
    Divide el trabajo entre varios hilos.
    """
    resultados = [0] * num_hilos
    hilos = []

    # Tamaño de cada trozo de trabajo
    chunk = n // num_hilos

    def tarea_hilo(id_hilo: int, inicio: int, fin: int):
        resultados[id_hilo] = suma_cuadrados(inicio, fin)

    # Crear e iniciar hilos
    for i in range(num_hilos):
        inicio = i * chunk
        fin = (i + 1) * chunk if i < num_hilos - 1 else n
        hilo = threading.Thread(target=tarea_hilo, args=(i, inicio, fin))
        hilos.append(hilo)
        hilo.start()

    # Esperar a que todos los hilos terminen
    for hilo in hilos:
        hilo.join()

    return sum(resultados)

# 3. ENFOQUE CON MULTIPROCESSING (Paralelismo real)
def con_multiprocessing(n: int, num_procesos: int = None) -> int:
    """
    Usa múltiples procesos del sistema operativo.
    Evita el GIL → paralelismo REAL en tareas CPU-bound.
    Cada proceso tiene su propia memoria (memoria distribuida local).
    """
    if num_procesos is None:
        num_procesos = multiprocessing.cpu_count()

    chunk = n // num_procesos
    rangos = []

    for i in range(num_procesos):
        inicio = i * chunk
        fin = (i + 1) * chunk if i < num_procesos - 1 else n
        rangos.append((inicio, fin))

    # Pool de procesos: distribuye el trabajo automáticamente
    with multiprocessing.Pool(processes=num_procesos) as pool:
        resultados = pool.starmap(suma_cuadrados, rangos)

    return sum(resultados)

# FUNCIÓN DE BENCHMARK
def benchmark(n: int):
    """Compara los tres enfoques midiendo tiempo de ejecución."""
    cpus = multiprocessing.cpu_count()
    print(f"\n{'='*60}")
    print(f"  BENCHMARK: Suma de cuadrados de 0 a {n:,}")
    print(f"  CPUs disponibles: {cpus}")
    print(f"{'='*60}")

    # Secuencial 
    t0 = time.perf_counter()
    res_sec = secuencial(n)
    t_sec = time.perf_counter() - t0
    print(f"\n[1] Secuencial")
    print(f"    Resultado : {res_sec:,}")
    print(f"    Tiempo    : {t_sec:.4f} s")

    #  Threading
    t0 = time.perf_counter()
    res_thr = con_threading(n, num_hilos=cpus)
    t_thr = time.perf_counter() - t0
    print(f"\n[2] Threading ({cpus} hilos)")
    print(f"    Resultado : {res_thr:,}")
    print(f"    Tiempo    : {t_thr:.4f} s")
    print(f"    Speedup   : {t_sec/t_thr:.2f}x")

    #  Multiprocessing 
    t0 = time.perf_counter()
    res_mp = con_multiprocessing(n, num_procesos=cpus)
    t_mp = time.perf_counter() - t0
    print(f"\n[3] Multiprocessing ({cpus} procesos)")
    print(f"    Resultado : {res_mp:,}")
    print(f"    Tiempo    : {t_mp:.4f} s")
    print(f"    Speedup   : {t_sec/t_mp:.2f}x")

    # Verificación de corrección 
    print(f"\n{'─'*60}")
    if res_sec == res_thr == res_mp:
        print("  ✓ VERIFICACIÓN: Todos los métodos produjeron el mismo resultado.")
    else:
        print("  ✗ ERROR: Los resultados no coinciden.")

    print(f"\n  Resumen de Speedup:")
    print(f"    Threading vs Secuencial    : {t_sec/t_thr:.2f}x más rápido")
    print(f"    Multiprocessing vs Secuenc : {t_sec/t_mp:.2f}x más rápido")
    print(f"{'='*60}\n")



#  Productor-Consumidor con Queue. Ejemplo 2

def demo_productor_consumidor():
    """
    Patrón clásico de concurrencia usando threading.Queue.
    El Productor genera tareas, el Consumidor las procesa.
    """
    from queue import Queue

    cola = Queue(maxsize=10)
    resultados_pc = []
    lock = threading.Lock()

    def productor(cola: Queue, items: int):
        for i in range(items):
            cola.put(i)
            print(f"  → Productor envió: {i}")
        cola.put(None)  # Señal de fin

    def consumidor(cola: Queue):
        while True:
            item = cola.get()
            if item is None:
                break
            resultado = item ** 2
            with lock:
                resultados_pc.append(resultado)
            print(f"  ← Consumidor procesó: {item}² = {resultado}")
            cola.task_done()

    print("\n[DEMO] Patrón Productor-Consumidor con Threading.Queue")
    print(f"{'─'*50}")

    hilo_prod = threading.Thread(target=productor, args=(cola, 5))
    hilo_cons = threading.Thread(target=consumidor, args=(cola,))

    hilo_prod.start()
    hilo_cons.start()

    hilo_prod.join()
    hilo_cons.join()

    print(f"\n  Resultados acumulados: {resultados_pc}")
    print(f"{'─'*50}")


# PUNTO DE ENTRADA
if __name__ == "__main__":
    print("\n" + "="*60)
    print("  COMPUTACIÓN PARALELA Y DISTRIBUIDA EN PYTHON")
    print("  Módulos: threading | multiprocessing")
    print("="*60)

    # Benchmark principal con 5 millones de elementos
    N = 5_000_000
    benchmark(N)

    # Demo del patrón Productor-Consumidor
    demo_productor_consumidor()

    print("\nPrograma finalizado correctamente.\n")
