@echo off
REM NotesApp Build Script for Windows
REM This script automates the build process on Windows

setlocal enabledelayedexpansion

REM Colors (limited support in Windows batch)
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

REM Function to print colored output
:print_status
echo %BLUE%[INFO]%NC% %~1
goto :eof

:print_success
echo %GREEN%[SUCCESS]%NC% %~1
goto :eof

:print_warning
echo %YELLOW%[WARNING]%NC% %~1
goto :eof

:print_error
echo %RED%[ERROR]%NC% %~1
goto :eof

REM Check if command exists
:command_exists
where %1 >nul 2>&1
if %errorlevel% equ 0 (
    exit /b 0
) else (
    exit /b 1
)

REM Check dependencies
:check_dependencies
call :print_status "Checking dependencies..."

set "missing_deps="

REM Check for CMake
call :command_exists cmake
if %errorlevel% neq 0 (
    set "missing_deps=!missing_deps! cmake"
)

REM Check for Qt (try different paths)
set "qt_found=0"
if exist "C:\Qt\6.5.0\mingw_64\bin\qmake.exe" set "qt_found=1"
if exist "C:\Qt\6.6.0\mingw_64\bin\qmake.exe" set "qt_found=1"
if exist "C:\Qt\6.7.0\mingw_64\bin\qmake.exe" set "qt_found=1"
if exist "C:\Qt\6.8.0\mingw_64\bin\qmake.exe" set "qt_found=1"
if exist "C:\Qt\6.9.0\mingw_64\bin\qmake.exe" set "qt_found=1"

if %qt_found% equ 0 (
    set "missing_deps=!missing_deps! qt6"
)

REM Check for compiler
call :command_exists g++
if %errorlevel% neq 0 (
    call :command_exists cl
    if %errorlevel% neq 0 (
        set "missing_deps=!missing_deps! c++ compiler"
    )
)

if not "!missing_deps!"=="" (
    call :print_error "Missing dependencies:!missing_deps!"
    call :print_status "Please install the missing dependencies and try again."
    exit /b 1
)

call :print_success "All dependencies found"
goto :eof

REM Setup build directory
:setup_build
call :print_status "Setting up build directory..."

if exist "build" (
    call :print_warning "Build directory already exists. Cleaning..."
    rmdir /s /q build
)

mkdir build
cd build
call :print_success "Build directory created"
goto :eof

REM Configure with CMake
:configure_cmake
call :print_status "Configuring with CMake..."

REM Try to find Qt installation
set "qt_path="
if exist "C:\Qt\6.9.0\mingw_64" set "qt_path=C:\Qt\6.9.0\mingw_64"
if exist "C:\Qt\6.8.0\mingw_64" set "qt_path=C:\Qt\6.8.0\mingw_64"
if exist "C:\Qt\6.7.0\mingw_64" set "qt_path=C:\Qt\6.7.0\mingw_64"
if exist "C:\Qt\6.6.0\mingw_64" set "qt_path=C:\Qt\6.6.0\mingw_64"
if exist "C:\Qt\6.5.0\mingw_64" set "qt_path=C:\Qt\6.5.0\mingw_64"

set "cmake_args="
if not "!qt_path!"=="" (
    set "cmake_args=-DCMAKE_PREFIX_PATH=!qt_path!"
)

REM Try different generators
call :command_exists ninja
if %errorlevel% equ 0 (
    set "cmake_args=!cmake_args! -G Ninja"
) else (
    call :command_exists mingw32-make
    if %errorlevel% equ 0 (
        set "cmake_args=!cmake_args! -G \"MinGW Makefiles\""
    ) else (
        set "cmake_args=!cmake_args! -G \"Visual Studio 17 2022\""
    )
)

REM Run CMake configuration
cmake .. %cmake_args%

if %errorlevel% equ 0 (
    call :print_success "CMake configuration completed"
) else (
    call :print_error "CMake configuration failed"
    exit /b 1
)
goto :eof

REM Build the project
:build_project
call :print_status "Building project..."

call :command_exists ninja
if %errorlevel% equ 0 (
    ninja
) else (
    call :command_exists mingw32-make
    if %errorlevel% equ 0 (
        mingw32-make -j4
    ) else (
        cmake --build . --config Release
    )
)

if %errorlevel% equ 0 (
    call :print_success "Build completed successfully"
) else (
    call :print_error "Build failed"
    exit /b 1
)
goto :eof

REM Run the application
:run_app
call :print_status "Running NotesApp..."

if exist "NotesApp.exe" (
    call :print_success "Starting NotesApp..."
    NotesApp.exe
) else (
    call :print_error "Executable not found: NotesApp.exe"
    exit /b 1
)
goto :eof

REM Clean build
:clean_build
call :print_status "Cleaning build directory..."
if exist "build" (
    rmdir /s /q build
    call :print_success "Build directory cleaned"
) else (
    call :print_warning "No build directory to clean"
)
goto :eof

REM Show help
:show_help
echo NotesApp Build Script for Windows
echo.
echo Usage: %0 [OPTION]
echo.
echo Options:
echo   build     Build the project (default)
echo   run       Build and run the application
echo   clean     Clean the build directory
echo   help      Show this help message
echo.
echo Examples:
echo   %0 build    # Build the project
echo   %0 run      # Build and run
echo   %0 clean    # Clean build files
goto :eof

REM Main function
:main
set "action=%~1"
if "%action%"=="" set "action=build"

if "%action%"=="build" (
    call :check_dependencies
    call :setup_build
    call :configure_cmake
    call :build_project
    call :print_success "Build completed! Run 'build.bat run' to start the application."
) else if "%action%"=="run" (
    call :check_dependencies
    call :setup_build
    call :configure_cmake
    call :build_project
    call :run_app
) else if "%action%"=="clean" (
    call :clean_build
) else if "%action%"=="help" (
    call :show_help
) else if "%action%"=="-h" (
    call :show_help
) else if "%action%"=="--help" (
    call :show_help
) else (
    call :print_error "Unknown action: %action%"
    call :show_help
    exit /b 1
)

goto :eof

REM Run main function
call :main %*
