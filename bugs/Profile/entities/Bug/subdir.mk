################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../entities/Bug/LifetimeSensor.cpp 

OBJS += \
./entities/Bug/LifetimeSensor.o 

CPP_DEPS += \
./entities/Bug/LifetimeSensor.d 


# Each subdirectory must supply rules for building sources it contributes
entities/Bug/%.o: ../entities/Bug/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -I/home/bog/work/bugs/3rdparty/easyunit -O2 -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


