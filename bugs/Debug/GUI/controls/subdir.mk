################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../GUI/controls/Button.cpp \
../GUI/controls/TextField.cpp 

OBJS += \
./GUI/controls/Button.o \
./GUI/controls/TextField.o 

CPP_DEPS += \
./GUI/controls/Button.d \
./GUI/controls/TextField.d 


# Each subdirectory must supply rules for building sources it contributes
GUI/controls/Button.o: /mnt/docs/Work/bugs/bugs/GUI/controls/Button.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

GUI/controls/TextField.o: /mnt/docs/Work/bugs/bugs/GUI/controls/TextField.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


