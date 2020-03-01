################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ai/Chromosome.cpp \
../src/ai/FuzzyController.cpp \
../src/ai/GAManagerModule.cpp

OBJS += \
./src/ai/Chromosome.o \
./src/ai/FuzzyController.o \
./src/ai/GAManagerModule.o

CPP_DEPS += \
./src/ai/Chromosome.d \
./src/ai/FuzzyController.d \
./src/ai/GAManagerModule.d


# Each subdirectory must supply rules for building sources it contributes
src/gui/%.o: ../src/gui/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/OGRE -I/usr/include/OIS -I/usr/local/include/fmodex -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


