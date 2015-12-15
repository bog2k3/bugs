################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../neuralnet/Network.cpp \
../neuralnet/Neuron.cpp \
../neuralnet/OutputSocket.cpp \
../neuralnet/Traverser.cpp \
../neuralnet/functions.cpp 

OBJS += \
./neuralnet/Network.o \
./neuralnet/Neuron.o \
./neuralnet/OutputSocket.o \
./neuralnet/Traverser.o \
./neuralnet/functions.o 

CPP_DEPS += \
./neuralnet/Network.d \
./neuralnet/Neuron.d \
./neuralnet/OutputSocket.d \
./neuralnet/Traverser.d \
./neuralnet/functions.d 


# Each subdirectory must supply rules for building sources it contributes
neuralnet/%.o: ../neuralnet/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


