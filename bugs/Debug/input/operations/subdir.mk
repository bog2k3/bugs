################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../input/operations/OperationGui.cpp \
../input/operations/OperationPan.cpp \
../input/operations/OperationSpring.cpp \
../input/operations/OperationsStack.cpp 

OBJS += \
./input/operations/OperationGui.o \
./input/operations/OperationPan.o \
./input/operations/OperationSpring.o \
./input/operations/OperationsStack.o 

CPP_DEPS += \
./input/operations/OperationGui.d \
./input/operations/OperationPan.d \
./input/operations/OperationSpring.d \
./input/operations/OperationsStack.d 


# Each subdirectory must supply rules for building sources it contributes
input/operations/OperationGui.o: /mnt/docs/Work/bugs/bugs/input/operations/OperationGui.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

input/operations/OperationPan.o: /mnt/docs/Work/bugs/bugs/input/operations/OperationPan.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

input/operations/OperationSpring.o: /mnt/docs/Work/bugs/bugs/input/operations/OperationSpring.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

input/operations/OperationsStack.o: /mnt/docs/Work/bugs/bugs/input/operations/OperationsStack.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


