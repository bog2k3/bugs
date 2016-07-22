################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../genetics/Gene.cpp \
../genetics/Genome.cpp \
../genetics/Ribosome.cpp 

OBJS += \
./genetics/Gene.o \
./genetics/Genome.o \
./genetics/Ribosome.o 

CPP_DEPS += \
./genetics/Gene.d \
./genetics/Genome.d \
./genetics/Ribosome.d 


# Each subdirectory must supply rules for building sources it contributes
genetics/%.o: ../genetics/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -I"/mnt/docs/Work/bugs/bugs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


