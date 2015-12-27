################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../serialization/BigFile.cpp \
../serialization/BinaryStream.cpp \
../serialization/ChromosomeSerialization.cpp \
../serialization/GeneSerialization.cpp \
../serialization/GenomeSerialization.cpp \
../serialization/Serializer.cpp 

OBJS += \
./serialization/BigFile.o \
./serialization/BinaryStream.o \
./serialization/ChromosomeSerialization.o \
./serialization/GeneSerialization.o \
./serialization/GenomeSerialization.o \
./serialization/Serializer.o 

CPP_DEPS += \
./serialization/BigFile.d \
./serialization/BinaryStream.d \
./serialization/ChromosomeSerialization.d \
./serialization/GeneSerialization.d \
./serialization/GenomeSerialization.d \
./serialization/Serializer.d 


# Each subdirectory must supply rules for building sources it contributes
serialization/%.o: ../serialization/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


