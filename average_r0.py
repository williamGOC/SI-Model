#!/usr/bin/env python3
"""
Script para procesar archivos .dat en un directorio.
Para cada .dat, extrae parámetros del nombre, calcula media y error de la última columna,
y genera un archivo MEAN_VALUES.dat con estructura:
phi, rc, N, alpha, sigma, beta, lambda, mean_R0, stderr_R0
"""

import os
import sys
import re
import numpy as np
import pandas as pd
from pathlib import Path

def extract_parameters(filename):
    """
    Extrae parámetros del nombre del archivo .dat
    Formato esperado: data_phi{PHI}_rc{RC}_N{N}_alpha{ALPHA}_sigma{SIGMA}_beta{BETA}_lambda{LAMBDA}.dat
    """
    # Patrón para extraer parámetros
    pattern = r'data_phi([\d.]+)_rc([\d.]+)_N(\d+)_alpha([\d.]+)_sigma([\d.]+)_beta([\d.]+)_lambda([\d.]+)\.dat'
    
    match = re.search(pattern, filename)
    if not match:
        return None
    
    params = {
        'phi': float(match.group(1)),
        'rc': float(match.group(2)),
        'N': int(match.group(3)),
        'alpha': float(match.group(4)),
        'sigma': float(match.group(5)),
        'beta': float(match.group(6)),
        'lambda': float(match.group(7))
    }
    return params

def read_last_column(filepath):
    """
    Lee archivo .dat y extrae la última columna (numérica)
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
                if len(parts) > 0:
                    try:
                        # Extraer última columna
                        last_value = float(parts[-1])
                        data.append(last_value)
                    except (ValueError, IndexError):
                        continue
    except Exception as e:
        print(f"Error leyendo {filepath}: {e}")
        return None
    
    if len(data) == 0:
        return None
    
    return np.array(data)

def calculate_stats(data):
    """
    Calcula media y error estándar
    """
    if data is None or len(data) == 0:
        return np.nan, np.nan
    
    mean = np.mean(data)
    stderr = np.std(data) / np.sqrt(len(data))
    
    return mean, stderr

def process_directory(directory):
    """
    Procesa todos los archivos .dat en el directorio
    """
    results = []
    dat_files = sorted(Path(directory).glob('data_*.dat'))
    
    if len(dat_files) == 0:
        print(f"Error: No se encontraron archivos data_*.dat en {directory}")
        return None
    
    print(f"Procesando {len(dat_files)} archivos...")
    
    for i, filepath in enumerate(dat_files):
        filename = filepath.name
        
        # Extraer parámetros del nombre
        params = extract_parameters(filename)
        if params is None:
            print(f"⚠ No se pudieron extraer parámetros de: {filename}")
            continue
        
        # Leer datos (última columna)
        data = read_last_column(filepath)
        if data is None:
            print(f"⚠ No se pudieron leer datos de: {filename}")
            continue
        
        # Calcular estadísticas
        mean_val, stderr_val = calculate_stats(data)
        
        # Agregar fila de resultados
        row = {
            'phi': params['phi'],
            'rc': params['rc'],
            'N': params['N'],
            'alpha': params['alpha'],
            'sigma': params['sigma'],
            'beta': params['beta'],
            'lambda': params['lambda'],
            'mean_R0': mean_val,
            'stderr_R0': stderr_val
        }
        results.append(row)
        
        print(f"✓ [{i+1}/{len(dat_files)}] {filename}")
        print(f"  -> mean_R0={mean_val:.6f}, stderr_R0={stderr_val:.6f}")
    
    if len(results) == 0:
        print("Error: No se procesaron archivos exitosamente")
        return None
    
    # Convertir a DataFrame
    df = pd.DataFrame(results)
    
    # Ordenar por parámetros
    df = df.sort_values(['phi', 'rc', 'N', 'alpha', 'sigma', 'beta', 'lambda'])
    
    return df

def main():
    if len(sys.argv) < 2:
        directory = "./R0"
        print(f"Uso: python3 generate_mean_values.py [directorio]")
        print(f"Usando directorio por defecto: {directory}")
    else:
        directory = sys.argv[1]
    
    if not os.path.isdir(directory):
        print(f"Error: El directorio '{directory}' no existe")
        sys.exit(1)
    
    # Procesar directorio
    df = process_directory(directory)
    
    if df is None:
        sys.exit(1)
    
    # Generar nombre del archivo de salida
    output_file = os.path.join(directory, "MEAN_VALUES.dat")
    
    # Guardar como archivo de texto (.dat) con separadores
    with open(output_file, 'w') as f:
        # Escribir encabezado
        f.write("# phi\trc\tN\talpha\tsigma\tbeta\tlambda\tmean_R0\tstderr_R0\n")
        
        # Escribir datos
        for _, row in df.iterrows():
            f.write(f"{row['phi']:.6f}\t{row['rc']:.6f}\t{row['N']}\t{row['alpha']:.6f}\t")
            f.write(f"{row['sigma']:.6f}\t{row['beta']:.6f}\t{row['lambda']:.6f}\t")
            f.write(f"{row['mean_R0']:.6f}\t{row['stderr_R0']:.6f}\n")
    
    print(f"\n# ========================================")
    print(f"# ✓ Resultados guardados en: {output_file}")
    print(f"# ========================================")
    print(f"\nResumen de datos:")
    print(df.to_string(index=False))
    
    return output_file

if __name__ == "__main__":
    main()