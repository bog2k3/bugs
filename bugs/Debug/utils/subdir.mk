################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../utils/log.cpp \
../utils/rand.cpp 

OBJS += \
./utils/log.o \
./utils/rand.o 

CPP_DEPS += \
./utils/log.d \
./utils/rand.d 


# Each subdirectory must supply rules for building sources it contributes
utils/%.o: ../utils/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


