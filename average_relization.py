#!/usr/bin/env python3
"""
Script para procesar archivos de realizaciones en cada carpeta lambda_X.XXXX/
Para cada paso de tiempo, promedia S e I entre todas las realizaciones
Genera archivo MEAN_TEMPORAL.dat con estructura:
Step  Time  mean_S  stderr_S  mean_I  stderr_I
"""

import os
import sys
import numpy as np
import pandas as pd
from pathlib import Path
from collections import defaultdict

def read_simulation_file(filepath):
    """
    Lee archivo de simulación y extrae Step, Time, S, I
    Salta líneas de comentarios
    """
    data = []
    
    try:
        with open(filepath, 'r') as f:
            for line in f:
                # Saltar líneas de comentarios
                if line.startswith('#'):
                    continue
                
                # Parsear línea de datos
                parts = line.strip().split()
                if len(parts) >= 4:
                    try:
                        # Formato esperado: Step  Time  S  I
                        step = int(parts[0])
                        time = float(parts[1])
                        s = int(parts[2])
                        i = int(parts[3])
                        data.append({
                            'step': step,
                            'time': time,
                            'S': s,
                            'I': i
                        })
                    except (ValueError, IndexError):
                        continue
    except Exception as e:
        print(f"Error leyendo {filepath}: {e}")
        return None
    
    if len(data) == 0:
        return None
    
    return data

def process_lambda_directory(lambda_dir):
    """
    Procesa todos los archivos de realizaciones en una carpeta lambda_X.XXXX/
    """
    # Encontrar todos los archivos de realización
    real_files = sorted(Path(lambda_dir).glob('lambda_*_real_*.dat'))
    
    if len(real_files) == 0:
        print(f"⚠ No se encontraron archivos de realización en {lambda_dir}")
        return None
    
    print(f"  Procesando {len(real_files)} realizaciones...")
    
    # Diccionario para almacenar datos por step
    time_data = defaultdict(lambda: {'S': [], 'I': [], 'time': None})
    
    # Leer todos los archivos de realización
    for real_file in real_files:
        data = read_simulation_file(real_file)
        if data is None:
            continue
        
        # Agregar datos a time_data
        for row in data:
            step = row['step']
            time_data[step]['S'].append(row['S'])
            time_data[step]['I'].append(row['I'])
            time_data[step]['time'] = row['time']  # Todos tienen el mismo time para un step
    
    # Calcular media y error para cada step
    results = []
    for step in sorted(time_data.keys()):
        s_values = np.array(time_data[step]['S'])
        i_values = np.array(time_data[step]['I'])
        time = time_data[step]['time']
        
        mean_s = np.mean(s_values)
        stderr_s = np.std(s_values) / np.sqrt(len(s_values))
        
        mean_i = np.mean(i_values)
        stderr_i = np.std(i_values) / np.sqrt(len(i_values))
        
        results.append({
            'step': step,
            'time': time,
            'mean_S': mean_s,
            'stderr_S': stderr_s,
            'mean_I': mean_i,
            'stderr_I': stderr_i
        })
    
    return pd.DataFrame(results)

def main():
    if len(sys.argv) < 2:
        base_dir = "./Temp"
        print(f"Uso: python3 average_realizations.py [directorio_base]")
        print(f"Usando directorio por defecto: {base_dir}")
    else:
        base_dir = sys.argv[1]
    
    if not os.path.isdir(base_dir):
        print(f"Error: El directorio '{base_dir}' no existe")
        sys.exit(1)
    
    # Encontrar todas las carpetas lambda_X.XXXX/
    lambda_dirs = sorted([d for d in Path(base_dir).iterdir() if d.is_dir() and d.name.startswith('lambda_')])
    
    if len(lambda_dirs) == 0:
        print(f"Error: No se encontraron directorios lambda_* en {base_dir}")
        sys.exit(1)
    
    print(f"Procesando {len(lambda_dirs)} carpetas de lambda...")
    print()
    
    # Procesar cada carpeta lambda
    for lambda_dir in lambda_dirs:
        lambda_name = lambda_dir.name
        print(f"✓ {lambda_name}")
        
        # Procesar realizaciones
        df = process_lambda_directory(lambda_dir)
        
        if df is None:
            print(f"  ⚠ No se procesó correctamente")
            continue
        
        # Generar nombre del archivo de salida
        output_file = os.path.join(lambda_dir, "MEAN_TEMPORAL.dat")
        
        # Guardar como archivo de texto (.dat) con separadores
        with open(output_file, 'w') as f:
            # Escribir encabezado
            f.write("# Step\tTime\tmean_S\tstderr_S\tmean_I\tstderr_I\n")
            
            # Escribir datos
            for _, row in df.iterrows():
                f.write(f"{int(row['step'])}\t{row['time']:.4f}\t")
                f.write(f"{row['mean_S']:.6f}\t{row['stderr_S']:.6f}\t")
                f.write(f"{row['mean_I']:.6f}\t{row['stderr_I']:.6f}\n")
        
        print(f"  → {output_file}")
        print(f"  → {len(df)} pasos de tiempo promediados")
        print()
    
    print("# ========================================")
    print("# ✓ Procesamiento completado!")
    print("# ========================================")

if __name__ == "__main__":
    main()