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
session/%.o: ../session/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL -DDEBUG -I../3rdparty/easyunit -I/home/bog/work/boglfw/build/dist/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


