#!/bin/bash

BUILD_TYPE="Debug"
CLEAN=false
JOBS=12

show_help() {
    echo "Uso: $0 [opciones]"
    echo "Opciones:"
    echo "  -c, --clean    Limpiar directorio de build antes de compilar"
    echo "  -r, --release  Compilar en modo Release"
    echo "  -d, --debug    Compilar en modo Debug (por defecto)"
    echo "  -h, --help     Mostrar esta ayuda"
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
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
else
    BUILD_DIR="build/debug"
fi

echo "┌────────────────────────────────────────┐"
echo "│        INFORMACION DE COMPILACION      │"
echo "├────────────────────────────────────────┤"
echo "│ Build Type:    $BUILD_TYPE"
echo "│ Jobs:          $JOBS"
echo "│ Build Dir:     $BUILD_DIR"

if command -v g++ &> /dev/null; then
    COMPILER="g++ $(g++ --version | head -n1 | grep -oP '\d+\.\d+\.\d+')"
elif command -v clang++ &> /dev/null; then
    COMPILER="clang++ $(clang++ --version | head -n1 | grep -oP '\d+\.\d+\.\d+')"
else
    COMPILER="Desconocido"
fi
echo "│ Compilador:    $COMPILER"

if [[ -f "CMakeLists.txt" ]]; then
    if [[ "$BUILD_TYPE" == "Release" ]]; then
        CMAKE_FLAGS=$(grep -i "CMAKE_CXX_FLAGS_RELEASE" CMakeLists.txt | head -1 | sed 's/.*CMAKE_CXX_FLAGS_RELEASE[[:space:]]*[\"]*\([^\"]*\)[\"]*.*$/\1/')
        if [[ -z "$CMAKE_FLAGS" ]]; then
            CMAKE_FLAGS="-O3 -DNDEBUG"
        fi
    else
        CMAKE_FLAGS=$(grep -i "CMAKE_CXX_FLAGS_DEBUG" CMakeLists.txt | head -1 | sed 's/.*CMAKE_CXX_FLAGS_DEBUG[[:space:]]*[\"]*\([^\"]*\)[\"]*.*$/\1/')
        if [[ -z "$CMAKE_FLAGS" ]]; then
            CMAKE_FLAGS="-g -O0"
        fi
    fi
else
    if [[ "$BUILD_TYPE" == "Release" ]]; then
        CMAKE_FLAGS="-O3 -DNDEBUG"
    else
        CMAKE_FLAGS="-g -O0"
    fi
fi
echo "│ Flags:         $CMAKE_FLAGS"

if [[ -f "CMakeLists.txt" ]]; then
    CPP_STANDARD=$(grep -i "CMAKE_CXX_STANDARD" CMakeLists.txt | head -1 | sed 's/.*CMAKE_CXX_STANDARD[[:space:]]*\([0-9]*\).*/\1/')
    if [[ -n "$CPP_STANDARD" ]]; then
        echo "│ C++ Standard:  C++$CPP_STANDARD"
    else
        echo "│ C++ Standard:  No especificado"
    fi
else
    echo "│ C++ Standard:  CMakeLists.txt no encontrado"
fi

echo "└────────────────────────────────────────┘"

if [[ "$CLEAN" == true ]]; then
    echo ""
    echo "┌─── LIMPIEZA ───┐"
    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
        echo "│ Directorio limpiado: $BUILD_DIR"
    else
        echo "│ Nada que limpiar"
    fi
    echo "└───────────────┘"
    echo ""
fi

if [[ ! -d "$BUILD_DIR" ]]; then
    echo ""
    echo "┌─── CONFIGURACION CMAKE ───┐"
    cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    
    if [[ $? -ne 0 ]]; then
        echo "│ ERROR: Configuracion fallida"
        exit 1
    fi
    echo "│ CMake configurado"
    echo "└───────────────────────────┘"
    echo ""
fi

echo ""
echo "┌─── COMPILACION ───┐"
echo "│ cmake --build $BUILD_DIR -j $JOBS"
echo "└───────────────────┘"
echo ""

cmake --build "$BUILD_DIR" -j "$JOBS"

if [[ $? -eq 0 ]]; then
    echo ""
    echo "┌─── RESULTADO ───┐"
    echo "│ EXITO: Compilacion completada"
    echo "│ Ejecutables en: $BUILD_DIR"
    echo "└─────────────────┘"
else
    echo ""
    echo "┌─── RESULTADO ───┐"
    echo "│ ERROR: Compilacion fallida"
    echo "└─────────────────┘"
    exit 1
fi