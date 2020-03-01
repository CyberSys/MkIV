################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/scenery/ETM/ETBrush.cpp \
../src/scenery/ETM/ETIndexHandler.cpp \
../src/scenery/ETM/ETLightmap.cpp \
../src/scenery/ETM/ETLoadSaveHeightmap.cpp \
../src/scenery/ETM/ETSplattingManager.cpp \
../src/scenery/ETM/ETTerrainInfo.cpp \
../src/scenery/ETM/ETTerrainManager.cpp \
../src/scenery/ETM/ETTile.cpp

OBJS += \
./src/scenery/ETM/ETBrush.o \
./src/scenery/ETM/ETIndexHandler.o \
./src/scenery/ETM/ETLightmap.o \
./src/scenery/ETM/ETLoadSaveHeightmap.o \
./src/scenery/ETM/ETSplattingManager.o \
./src/scenery/ETM/ETTerrainInfo.o \
./src/scenery/ETM/ETTerrainManager.o \
./src/scenery/ETM/ETTile.o

CPP_DEPS += \
./src/scenery/ETM/ETBrush.d \
./src/scenery/ETM/ETIndexHandler.d \
./src/scenery/ETM/ETLightmap.d \
./src/scenery/ETM/ETLoadSaveHeightmap.d \
./src/scenery/ETM/ETSplattingManager.d \
./src/scenery/ETM/ETTerrainInfo.d \
./src/scenery/ETM/ETTerrainManager.d \
./src/scenery/ETM/ETTile.d

# Each subdirectory must supply rules for building sources it contributes
src/scenery/%.o: ../src/scenery/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/OGRE -I/usr/include/OIS -I/usr/local/include/fmodex -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


