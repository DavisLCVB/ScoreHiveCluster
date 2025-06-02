#!/bin/bash

BUILD_TYPE="Debug"
EXECUTABLE="ScoreHiveCluster"

show_help() {
    echo "Uso: $0 [opciones]"
    echo "Opciones:"
    echo "  -r, --release  Buscar en directorio Release"
    echo "  -d, --debug    Buscar en directorio Debug (por defecto)"
    echo "  -h, --help     Mostrar esta ayuda"
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

EXE_PATH="$BUILD_DIR/$EXECUTABLE"

if [[ ! -f "$EXE_PATH" ]]; then
    echo "Error: Ejecutable no encontrado: $EXE_PATH"
    exit 1
fi

if [[ ! -x "$EXE_PATH" ]]; then
    echo "Error: El archivo no es ejecutable: $EXE_PATH"
    exit 1
fi

echo "┌────────────────────────────────────────┐"
echo "│           INFORMACION EJECUTABLE       │"
echo "├────────────────────────────────────────┤"
echo "│ Archivo:       $(basename "$EXE_PATH")"
echo "│ Ruta:          $EXE_PATH"

FILE_SIZE=$(stat -c%s "$EXE_PATH" 2>/dev/null || stat -f%z "$EXE_PATH" 2>/dev/null)
if [[ -n "$FILE_SIZE" ]]; then
    if [[ $FILE_SIZE -gt 1048576 ]]; then
        SIZE_MB=$((FILE_SIZE / 1048576))
        echo "│ Tamaño:        ${SIZE_MB} MB"
    elif [[ $FILE_SIZE -gt 1024 ]]; then
        SIZE_KB=$((FILE_SIZE / 1024))
        echo "│ Tamaño:        ${SIZE_KB} KB"
    else
        echo "│ Tamaño:        ${FILE_SIZE} bytes"
    fi
fi

LAST_MODIFIED=$(stat -c%y "$EXE_PATH" 2>/dev/null || stat -f%Sm "$EXE_PATH" 2>/dev/null)
if [[ -n "$LAST_MODIFIED" ]]; then
    echo "│ Modificado:    $LAST_MODIFIED"
fi

if command -v file &> /dev/null; then
    FILE_INFO=$(file "$EXE_PATH")
    ARCH=$(echo "$FILE_INFO" | grep -oP '(x86-64|i386|ARM|aarch64)' | head -1)
    if [[ -n "$ARCH" ]]; then
        echo "│ Arquitectura:  $ARCH"
    fi
    
    if echo "$FILE_INFO" | grep -q "not stripped"; then
        echo "│ Símbolos:      Not stripped (debug)"
    elif echo "$FILE_INFO" | grep -q "stripped"; then
        echo "│ Símbolos:      Stripped (release)"
    fi
fi

if command -v ldd &> /dev/null; then
    DEPS_COUNT=$(ldd "$EXE_PATH" 2>/dev/null | grep -c "=>")
    if [[ $DEPS_COUNT -gt 0 ]]; then
        echo "│ Dependencias:  $DEPS_COUNT librerías"
    fi
fi

echo "└────────────────────────────────────────┘"

if command -v objdump &> /dev/null || command -v nm &> /dev/null; then
    echo ""
    echo "┌─── SIMBOLOS Y SECCIONES ───┐"
    
    if command -v objdump &> /dev/null; then
        TEXT_SIZE=$(objdump -h "$EXE_PATH" 2>/dev/null | grep " \.text " | awk '{print $3}')
        DATA_SIZE=$(objdump -h "$EXE_PATH" 2>/dev/null | grep " \.data " | awk '{print $3}')
        BSS_SIZE=$(objdump -h "$EXE_PATH" 2>/dev/null | grep " \.bss " | awk '{print $3}')
        
        if [[ -n "$TEXT_SIZE" ]]; then
            TEXT_DEC=$((0x$TEXT_SIZE))
            echo "│ Sección .text: $TEXT_DEC bytes"
        fi
        if [[ -n "$DATA_SIZE" ]]; then
            DATA_DEC=$((0x$DATA_SIZE))
            echo "│ Sección .data: $DATA_DEC bytes"
        fi
        if [[ -n "$BSS_SIZE" ]]; then
            BSS_DEC=$((0x$BSS_SIZE))
            echo "│ Sección .bss:  $BSS_DEC bytes"
        fi
    fi
    
    if command -v nm &> /dev/null; then
        SYMBOL_COUNT=$(nm "$EXE_PATH" 2>/dev/null | wc -l)
        if [[ $SYMBOL_COUNT -gt 0 ]]; then
            echo "│ Total símbolos: $SYMBOL_COUNT"
        fi
    fi
    
    echo "└────────────────────────────┘"
fi

if command -v ldd &> /dev/null; then
    echo ""
    echo "┌─── DEPENDENCIAS ───┐"
    ldd "$EXE_PATH" 2>/dev/null | head -10 | while read line; do
        if [[ -n "$line" ]]; then
            LIB_NAME=$(echo "$line" | awk '{print $1}')
            echo "│ $LIB_NAME"
        fi
    done
    
    TOTAL_DEPS=$(ldd "$EXE_PATH" 2>/dev/null | wc -l)
    if [[ $TOTAL_DEPS -gt 10 ]]; then
        echo "│ ... y $((TOTAL_DEPS - 10)) más"
    fi
    echo "└───────────────────┘"
fi