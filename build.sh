#!/usr/bin/env bash
set -e

# Add local toolchains (llvm-mingw) to PATH if present
export PATH="/home/encrize/.local/bin:/home/encrize/.local/opt/llvm-mingw/bin:$PATH"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

print_header() {
    echo -e "\n========================================================"
    echo "  $1"
    echo -e "========================================================\n"
}

build_linux() {
    print_header "Building Linux Native Standalone (Embedded Assets + LTO + Strip)..."
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j$(nproc)
    
    echo -e "\n[Linux Build Success!]"
    ls -lh build/fnaf
}

build_windows() {
    print_header "Building Windows Standalone Static .exe (Embedded Assets + LTO + Strip)..."
    
    # Check for mingw compiler
    if ! command -v x86_64-w64-mingw32-g++ &>/dev/null; then
        echo "Error: x86_64-w64-mingw32-g++ compiler not found!"
        exit 1
    fi

    # Clean legacy artifacts if any
    rm -f build-win/fnaf-static.exe build-win/fnaf-upx.exe build-win/fnaf-windows-x64.zip

    cmake -B build-win \
        -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-w64-mingw32.cmake \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build build-win -j$(nproc)

    echo -e "\n[Windows Standalone Static .exe Ready!]"
    echo "You only need this single file: build-win/fnaf.exe"
    ls -lh build-win/fnaf.exe
}

TARGET="${1:-linux}"

case "$TARGET" in
    linux)
        build_linux
        ;;
    win|windows)
        build_windows
        ;;
    all)
        build_linux
        build_windows
        print_header "Summary of Built Artifacts"
        echo "Linux standalone binary:"
        ls -lh build/fnaf
        echo ""
        echo "Windows standalone static .exe (100% self-contained, no dependencies):"
        ls -lh build-win/fnaf.exe
        ;;
    clean)
        print_header "Cleaning build directories..."
        rm -rf build build-win
        echo "Clean completed."
        ;;
    *)
        echo "Usage: $0 [linux|windows|all|clean]"
        exit 1
        ;;
esac
