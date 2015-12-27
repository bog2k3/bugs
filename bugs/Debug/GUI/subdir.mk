################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../GUI/GuiBasicElement.cpp \
../GUI/GuiContainerElement.cpp \
../GUI/GuiHelper.cpp \
../GUI/GuiSystem.cpp \
../GUI/GuiTheme.cpp \
../GUI/Window.cpp 

OBJS += \
./GUI/GuiBasicElement.o \
./GUI/GuiContainerElement.o \
./GUI/GuiHelper.o \
./GUI/GuiSystem.o \
./GUI/GuiTheme.o \
./GUI/Window.o 

CPP_DEPS += \
./GUI/GuiBasicElement.d \
./GUI/GuiContainerElement.d \
./GUI/GuiHelper.d \
./GUI/GuiSystem.d \
./GUI/GuiTheme.d \
./GUI/Window.d 


# Each subdirectory must supply rules for building sources it contributes
GUI/%.o: ../GUI/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


