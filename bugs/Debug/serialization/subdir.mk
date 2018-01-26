################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../serialization/ChromosomeSerialization.cpp \
../serialization/GeneSerialization.cpp \
../serialization/GenomeSerialization.cpp 

OBJS += \
./serialization/ChromosomeSerialization.o \
./serialization/GeneSerialization.o \
./serialization/GenomeSerialization.o 

CPP_DEPS += \
./serialization/ChromosomeSerialization.d \
./serialization/GeneSerialization.d \
./serialization/GenomeSerialization.d 


# Each subdirectory must supply rules for building sources it contributes
serialization/%.o: ../serialization/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL -DDEBUG -I../3rdparty/easyunit -I/home/bog/work/boglfw/build/dist/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


