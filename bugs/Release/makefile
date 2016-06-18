################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utils/subdir.mk
-include session/subdir.mk
-include serialization/subdir.mk
-include renderOpenGL/subdir.mk
-include physics/subdir.mk
-include neuralnet/subdir.mk
-include math/subdir.mk
-include input/operations/subdir.mk
-include input/subdir.mk
-include genetics/subdir.mk
-include entities/food/subdir.mk
-include entities/Bug/subdir.mk
-include entities/subdir.mk
-include body-parts/sensors/subdir.mk
-include body-parts/subdir.mk
-include OSD/subdir.mk
-include GUI/controls/subdir.mk
-include GUI/subdir.mk
-include BSP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: bugs

# Tool invocations
bugs: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	g++  -o "bugs" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(C_DEPS)$(CPP_DEPS) bugs
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
