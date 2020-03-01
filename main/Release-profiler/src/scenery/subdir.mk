################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/scenery/PagedGeoWrapper.cpp \
../src/scenery/TerrainHandler.cpp \
../src/scenery/Shrub.cpp \
../src/scenery/ShrubDef.cpp \
../src/scenery/Building.cpp \
../src/scenery/BuildingDef.cpp \
../src/scenery/ScenarioDetails.cpp \
../src/scenery/WaterPlane.cpp

OBJS += \
./src/scenery/PagedGeoWrapper.o \
./src/scenery/TerrainHandler.o \
./src/scenery/Shrub.o \
./src/scenery/ShrubDef.o \
./src/scenery/Building.o \
./src/scenery/BuildingDef.o \
./src/scenery/ScenarioDetails.o \
./src/scenery/WaterPlane.o

CPP_DEPS += \
./src/scenery/PagedGeoWrapper.d \
./src/scenery/TerrainHandler.d \
./src/scenery/Shrub.d \
./src/scenery/ShrubDef.d \
./src/scenery/Building.d \
./src/scenery/BuildingDef.d \
./src/scenery/ScenarioDetails.d \
./src/scenery/WaterPlane.d

# Each subdirectory must supply rules for building sources it contributes
src/scenery/%.o: ../src/scenery/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/OGRE -I/usr/local/include/OIS -I/usr/local/include/fmodex -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<" -pg
	@echo 'Finished building: $<'
	@echo ' '


