################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../body-parts/sensors/Nose.cpp 

OBJS += \
./body-parts/sensors/Nose.o 

CPP_DEPS += \
./body-parts/sensors/Nose.d 


# Each subdirectory must supply rules for building sources it contributes
body-parts/sensors/%.o: ../body-parts/sensors/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DDEBUG -I../3rdparty/easyunit -O0 -g3 -p -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


