################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/MarkIV.cpp \
../src/ScenarioEditor.cpp \
../src/CSVPlotExporter.cpp \
../src/FileScanner.cpp \
../src/usefulFuncs.cpp \
../src/main.cpp 

OBJS += \
./src/MarkIV.o \
./src/ScenarioEditor.o \
./src/CSVPlotExporter.o \
./src/FileScanner.o \
./src/usefulFuncs.o \
./src/main.o 

CPP_DEPS += \
./src/MarkIV.d \
./src/ScenarioEditor.d \
./src/CSVPlotExporter.d \
./src/FileScanner.d \
./src/usefulFuncs.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/OGRE -I/usr/include/OIS -I/usr/local/include/fmodex -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


