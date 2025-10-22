#!/bin/bash

# NotesApp Build Script
# This script automates the build process for different platforms

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to detect platform
detect_platform() {
    case "$(uname -s)" in
        Linux*)     PLATFORM="linux";;
        Darwin*)    PLATFORM="macos";;
        CYGWIN*)    PLATFORM="windows";;
        MINGW*)     PLATFORM="windows";;
        MSYS*)      PLATFORM="windows";;
        *)          PLATFORM="unknown";;
    esac
    echo $PLATFORM
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_deps=()
    
    # Check for CMake
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    # Check for Qt
    if ! command_exists qmake6 && ! command_exists qmake; then
        missing_deps+=("qt6")
    fi
    
    # Check for compiler
    if ! command_exists g++ && ! command_exists clang++ && ! command_exists cl; then
        missing_deps+=("c++ compiler")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please install the missing dependencies and try again."
        exit 1
    fi
    
    print_success "All dependencies found"
}

# Function to setup build directory
setup_build() {
    print_status "Setting up build directory..."
    
    if [ -d "build" ]; then
        print_warning "Build directory already exists. Cleaning..."
        rm -rf build
    fi
    
    mkdir build
    cd build
    print_success "Build directory created"
}

# Function to configure with CMake
configure_cmake() {
    print_status "Configuring with CMake..."
    
    local platform=$(detect_platform)
    local cmake_args=""
    
    case $platform in
        "linux")
            cmake_args="-DCMAKE_BUILD_TYPE=Release"
            ;;
        "macos")
            # Try to find Qt6 installation
            if [ -d "/opt/homebrew/lib/cmake/Qt6" ]; then
                cmake_args="-DCMAKE_PREFIX_PATH=/opt/homebrew/lib/cmake/Qt6"
            elif [ -d "/usr/local/lib/cmake/Qt6" ]; then
                cmake_args="-DCMAKE_PREFIX_PATH=/usr/local/lib/cmake/Qt6"
            fi
            cmake_args="$cmake_args -DCMAKE_BUILD_TYPE=Release"
            ;;
        "windows")
            # Try different generators
            if command_exists ninja; then
                cmake_args="-G Ninja"
            elif command_exists mingw32-make; then
                cmake_args="-G 'MinGW Makefiles'"
            else
                cmake_args="-G 'Visual Studio 17 2022'"
            fi
            ;;
    esac
    
    # Run CMake configuration
    eval "cmake .. $cmake_args"
    
    if [ $? -eq 0 ]; then
        print_success "CMake configuration completed"
    else
        print_error "CMake configuration failed"
        exit 1
    fi
}

# Function to build the project
build_project() {
    print_status "Building project..."
    
    local platform=$(detect_platform)
    local build_args=""
    
    case $platform in
        "linux"|"macos")
            # Use all available CPU cores
            build_args="-j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
            make $build_args
            ;;
        "windows")
            if command_exists ninja; then
                ninja
            elif command_exists mingw32-make; then
                mingw32-make -j4
            else
                cmake --build . --config Release
            fi
            ;;
    esac
    
    if [ $? -eq 0 ]; then
        print_success "Build completed successfully"
    else
        print_error "Build failed"
        exit 1
    fi
}

# Function to run the application
run_app() {
    print_status "Running NotesApp..."
    
    local platform=$(detect_platform)
    local executable=""
    
    case $platform in
        "linux"|"macos")
            executable="./NotesApp"
            ;;
        "windows")
            executable="./NotesApp.exe"
            ;;
    esac
    
    if [ -f "$executable" ]; then
        print_success "Starting NotesApp..."
        $executable
    else
        print_error "Executable not found: $executable"
        exit 1
    fi
}

# Function to clean build
clean_build() {
    print_status "Cleaning build directory..."
    if [ -d "build" ]; then
        rm -rf build
        print_success "Build directory cleaned"
    else
        print_warning "No build directory to clean"
    fi
}

# Function to show help
show_help() {
    echo "NotesApp Build Script"
    echo ""
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  build     Build the project (default)"
    echo "  run       Build and run the application"
    echo "  clean     Clean the build directory"
    echo "  help      Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 build    # Build the project"
    echo "  $0 run      # Build and run"
    echo "  $0 clean    # Clean build files"
}

# Main function
main() {
    local action=${1:-build}
    
    case $action in
        "build")
            check_dependencies
            setup_build
            configure_cmake
            build_project
            print_success "Build completed! Run './build.sh run' to start the application."
            ;;
        "run")
            check_dependencies
            setup_build
            configure_cmake
            build_project
            run_app
            ;;
        "clean")
            clean_build
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        *)
            print_error "Unknown action: $action"
            show_help
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"
