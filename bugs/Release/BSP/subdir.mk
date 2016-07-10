################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BSP/BSPNode.cpp \
../BSP/BSPTree.cpp 

OBJS += \
./BSP/BSPNode.o \
./BSP/BSPTree.o 

CPP_DEPS += \
./BSP/BSPNode.d \
./BSP/BSPTree.d 


# Each subdirectory must supply rules for building sources it contributes
BSP/%.o: ../BSP/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -I"/mnt/docs/Work/bugs/bugs" -O3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


