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
GUI/GuiBasicElement.o: /mnt/docs/Work/bugs/bugs/GUI/GuiBasicElement.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

GUI/GuiContainerElement.o: /mnt/docs/Work/bugs/bugs/GUI/GuiContainerElement.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

GUI/GuiHelper.o: /mnt/docs/Work/bugs/bugs/GUI/GuiHelper.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

GUI/GuiSystem.o: /mnt/docs/Work/bugs/bugs/GUI/GuiSystem.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

GUI/GuiTheme.o: /mnt/docs/Work/bugs/bugs/GUI/GuiTheme.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

GUI/Window.o: /mnt/docs/Work/bugs/bugs/GUI/Window.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


