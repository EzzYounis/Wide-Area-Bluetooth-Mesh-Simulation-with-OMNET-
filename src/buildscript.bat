@echo off
echo Cleaning previous builds...
cd src
make clean

echo Checking for INET...
if not exist "..\..\inet4.5" (
    echo Error: INET 4.5 not found at ..\..\inet4.5
    echo Please ensure INET is properly installed and available
    pause
    exit /b 1
)

echo Generating makefile with INET support...
opp_makemake -f --deep -KINET4_5_PROJ=../../inet4.5 -DINET_IMPORT -L$(INET4_5_PROJ)/src -lINET$(D)

echo Building the project...
make

if %errorlevel% equ 0 (
    echo Build successful!
    echo To run the simulation, use:
    echo cd ..\simulations
    echo ..\src\BluetoothMeshSimulation -n .;..\src
) else (
    echo Build failed. Please check the error messages above.
    pause
    exit /b 1
)

cd ..
pause