################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../OSD/Label.cpp \
../OSD/ScaleDisplay.cpp 

OBJS += \
./OSD/Label.o \
./OSD/ScaleDisplay.o 

CPP_DEPS += \
./OSD/Label.d \
./OSD/ScaleDisplay.d 


# Each subdirectory must supply rules for building sources it contributes
OSD/%.o: ../OSD/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


