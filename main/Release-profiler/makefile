################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/units/subdir.mk
-include src/scenery/pagedgeometry/subdir.mk
-include src/scenery/subdir.mk
-include src/scenery/ETM/subdir.mk
-include src/scenery/ETM/Impl/subdir.mk
-include src/network/subdir.mk
#-include src/network/Raknet/subdir.mk
-include src/subdir.mk
-include src/input/subdir.mk
-include src/gui/subdir.mk
-include src/audio/subdir.mk
-include src/ai/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: markiv

# Tool invocations
markiv: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -o"../markiv" $(OBJS) $(USER_OBJS) $(LIBS_DIR) $(LIBS) -pg
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(EXECUTABLES)$(CC_DEPS)$(C_DEPS)$(CPP_DEPS)$(CXX_DEPS)$(C_UPPER_DEPS) ../markiv
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
