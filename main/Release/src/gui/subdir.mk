################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/gui/TsunamiGUI.cpp \
../src/gui/MouseCursor.cpp \
../src/gui/LobbyMenu.cpp \
../src/gui/MainMenu.cpp \
../src/gui/AbstractWidget.cpp \
../src/gui/BigTitleWidget.cpp \
../src/gui/ButtonWidget.cpp \
../src/gui/EntryWidget.cpp \
../src/gui/FileBrowserWidget.cpp \
../src/gui/FileSaverWidget.cpp \
../src/gui/MeshBrowserWidget.cpp \
../src/gui/UnitBrowserWidget.cpp \
../src/gui/ShrubBrowserWidget.cpp \
../src/gui/BuildingBrowserWidget.cpp \
../src/gui/ImageButtonWidget.cpp \
../src/gui/ImageWidget.cpp \
../src/gui/LabelWidget.cpp \
../src/gui/LineGraphWidget.cpp \
../src/gui/ScenarioDetailsWidget.cpp \
../src/gui/TextEntryWidget.cpp \
../src/gui/ThreeDeeLines.cpp \
../src/gui/InfoBoxWidget.cpp \
../src/gui/YesNoBoxWidget.cpp

OBJS += \
./src/gui/MouseCursor.o \
./src/gui/TsunamiGUI.o \
./src/gui/MainMenu.o \
./src/gui/AbstractWidget.o \
./src/gui/BigTitleWidget.o \
./src/gui/ButtonWidget.o \
./src/gui/EntryWidget.o \
./src/gui/FileBrowserWidget.o \
./src/gui/FileSaverWidget.o \
./src/gui/MeshBrowserWidget.o \
./src/gui/UnitBrowserWidget.o \
./src/gui/ShrubBrowserWidget.o \
./src/gui/BuildingBrowserWidget.o \
./src/gui/ImageButtonWidget.o \
./src/gui/ImageWidget.o \
./src/gui/LabelWidget.o \
./src/gui/LineGraphWidget.o \
./src/gui/LobbyMenu.o \
./src/gui/ScenarioDetailsWidget.o \
./src/gui/TextEntryWidget.o \
./src/gui/ThreeDeeLines.o \
./src/gui/InfoBoxWidget.o \
./src/gui/YesNoBoxWidget.o

CPP_DEPS += \
./src/gui/MouseCursor.d \
./src/gui/TsunamiGUI.d \
./src/gui/MainMenu.d \
./src/gui/AbstractWidget.d \
./src/gui/BigTitleWidget.d \
./src/gui/ButtonWidget.d \
./src/gui/EntryWidget.d \
./src/gui/FileBrowserWidget.d \
./src/gui/FileSaverWidget.d \
./src/gui/MeshBrowserWidget.d \
./src/gui/UnitBrowserWidget.d \
./src/gui/ShrubBrowserWidget.d \
./src/gui/BuildingBrowserWidget.d \
./src/gui/ImageButtonWidget.d \
./src/gui/ImageWidget.d \
./src/gui/LabelWidget.d \
./src/gui/LineGraphWidget.d \
./src/gui/LobbyMenu.d \
./src/gui/ScenarioDetailsWidget.d \
./src/gui/TextEntryWidget.d \
./src/gui/ThreeDeeLines.d \
./src/gui/InfoBoxWidget.d \
./src/gui/YesNoBoxWidget.d

# Each subdirectory must supply rules for building sources it contributes
src/gui/%.o: ../src/gui/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/OGRE -I/usr/include/OIS -I/usr/local/include/fmodex -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


