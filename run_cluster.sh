#!/bin/bash

BUILD_TYPE="Debug"
PROCESSES=4
EXECUTABLE="ScoreHiveCluster"

show_help() {
    echo "Uso: $0 [opciones]"
    echo "Opciones:"
    echo "  -r, --release      Ejecutar versión Release"
    echo "  -d, --debug        Ejecutar versión Debug (por defecto)"
    echo "  -n, --processes N  Número de procesos MPI (por defecto: 4)"
    echo "  -h, --help         Mostrar esta ayuda"
    echo ""
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -n|--processes)
            if [[ -n "$2" && "$2" =~ ^[0-9]+$ ]]; then
                PROCESSES="$2"
                shift 2
            else
                echo "Error: -n requiere un número válido"
                show_help
                exit 1
            fi
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "Opción desconocida: $1"
            show_help
            exit 1
            ;;
    esac
done

if [[ "$BUILD_TYPE" == "Release" ]]; then
    BUILD_DIR="build/release"
    ENV_DEBUG=""
else
    BUILD_DIR="build/debug"
    ENV_DEBUG="env DEBUG=1"
fi

EXE_PATH="$BUILD_DIR/$EXECUTABLE"

if [[ ! -f "$EXE_PATH" ]]; then
    echo "┌─── ERROR ───┐"
    echo "│ Ejecutable no encontrado: $EXE_PATH"
    echo "│ Compile primero con: ./build.sh"
    if [[ "$BUILD_TYPE" == "Release" ]]; then
        echo "│ Para release: ./build.sh -r"
    fi
    echo "└─────────────┘"
    exit 1
fi

if [[ ! -x "$EXE_PATH" ]]; then
    echo "┌─── ERROR ───┐"
    echo "│ El archivo no es ejecutable: $EXE_PATH"
    echo "└─────────────┘"
    exit 1
fi

if ! command -v mpirun &> /dev/null; then
    echo "┌─── ERROR ───┐"
    echo "│ mpirun no encontrado"
    echo "│ Instale MPI primero"
    echo "└─────────────┘"
    exit 1
fi

echo "┌────────────────────────────────────────┐"
echo "│            EJECUCION MPI               │"
echo "├────────────────────────────────────────┤"
echo "│ Build Type:    $BUILD_TYPE"
echo "│ Procesos:      $PROCESSES"
echo "│ Ejecutable:    $EXE_PATH"
if [[ "$BUILD_TYPE" == "Debug" ]]; then
    echo "│ Debug Mode:    Activado (DEBUG=1)"
else
    echo "│ Debug Mode:    Desactivado"
fi
echo "└────────────────────────────────────────┘"

echo ""
echo "┌─── COMANDO ───┐"
if [[ "$BUILD_TYPE" == "Debug" ]]; then
    echo "│ env DEBUG=1 mpirun -n $PROCESSES $EXE_PATH"
else
    echo "│ mpirun -n $PROCESSES $EXE_PATH"
fi
echo "└───────────────┘"
echo ""

if [[ "$BUILD_TYPE" == "Debug" ]]; then
    env DEBUG=1 mpirun -n "$PROCESSES" "$EXE_PATH"
else
    mpirun -n "$PROCESSES" "$EXE_PATH"
fi

EXIT_CODE=$?

echo ""
if [[ $EXIT_CODE -eq 0 ]]; then
    echo "┌─── RESULTADO ───┐"
    echo "│ Ejecución completada exitosamente"
    echo "└─────────────────┘"
else
    echo "┌─── RESULTADO ───┐"
    echo "│ Ejecución falló (código: $EXIT_CODE)"
    echo "└─────────────────┘"
    exit $EXIT_CODE
fi