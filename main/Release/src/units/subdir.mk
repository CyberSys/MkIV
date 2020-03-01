################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/units/Infantry.cpp \
../src/units/Projectile.cpp \
../src/units/Unit.cpp \
../src/units/UnitHandler.cpp \
../src/units/Vehicle.cpp \
../src/units/VehicleSpec.cpp \
../src/units/Amphibian.cpp \
../src/units/LandingCraft.cpp \
../src/units/GeneticLogger.cpp

OBJS += \
./src/units/Infantry.o \
./src/units/Projectile.o \
./src/units/Unit.o \
./src/units/UnitHandler.o \
./src/units/Vehicle.o \
./src/units/VehicleSpec.o \
./src/units/Amphibian.o \
./src/units/LandingCraft.o \
./src/units/GeneticLogger.o

CPP_DEPS += \
./src/units/Infantry.d \
./src/units/Projectile.d \
./src/units/Unit.d \
./src/units/UnitHandler.d \
./src/units/Vehicle.d \
./src/units/VehicleSpec.d \
./src/units/Amphibian.d \
./src/units/LandingCraft.d \
./src/units/GeneticLogger.d

# Each subdirectory must supply rules for building sources it contributes
src/units/%.o: ../src/units/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I /usr/include/OGRE -I/usr/include/OIS -I/usr/local/include/fmodex -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


