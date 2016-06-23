################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../session/PopulationManager.cpp \
../session/SessionManager.cpp 

OBJS += \
./session/PopulationManager.o \
./session/SessionManager.o 

CPP_DEPS += \
./session/PopulationManager.d \
./session/SessionManager.d 


# Each subdirectory must supply rules for building sources it contributes
session/PopulationManager.o: /mnt/docs/Work/bugs/bugs/session/PopulationManager.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

session/SessionManager.o: /mnt/docs/Work/bugs/bugs/session/SessionManager.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


