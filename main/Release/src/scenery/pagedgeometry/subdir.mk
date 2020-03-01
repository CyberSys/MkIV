################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/scenery/pagedgeometry/BatchedGeometry.cpp \
../src/scenery/pagedgeometry/BatchPage.cpp \
../src/scenery/pagedgeometry/GrassLoader.cpp \
../src/scenery/pagedgeometry/ImpostorPage.cpp \
../src/scenery/pagedgeometry/PagedGeometry.cpp \
../src/scenery/pagedgeometry/PropertyMaps.cpp \
../src/scenery/pagedgeometry/StaticBillboardSet.cpp \
../src/scenery/pagedgeometry/TreeLoader2D.cpp \
../src/scenery/pagedgeometry/TreeLoader3D.cpp

OBJS += \
./src/scenery/pagedgeometry/BatchedGeometry.o \
./src/scenery/pagedgeometry/BatchPage.o \
./src/scenery/pagedgeometry/GrassLoader.o \
./src/scenery/pagedgeometry/ImpostorPage.o \
./src/scenery/pagedgeometry/PagedGeometry.o \
./src/scenery/pagedgeometry/PropertyMaps.o \
./src/scenery/pagedgeometry/StaticBillboardSet.o \
./src/scenery/pagedgeometry/TreeLoader2D.o \
./src/scenery/pagedgeometry/TreeLoader3D.o

CPP_DEPS += \
./src/scenery/pagedgeometry/BatchedGeometry.d \
./src/scenery/pagedgeometry/BatchPage.d \
./src/scenery/pagedgeometry/GrassLoader.d \
./src/scenery/pagedgeometry/ImpostorPage.d \
./src/scenery/pagedgeometry/PagedGeometry.d \
./src/scenery/pagedgeometry/PropertyMaps.d \
./src/scenery/pagedgeometry/StaticBillboardSet.d \
./src/scenery/pagedgeometry/TreeLoader2D.d \
./src/scenery/pagedgeometry/TreeLoader3D.d

# Each subdirectory must supply rules for building sources it contributes
src/scenery/%.o: ../src/scenery/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/OGRE -I/usr/include/OIS -I/usr/local/include/fmodex -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


