################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../utils/ThreadPool.cpp \
../utils/UpdateList.cpp \
../utils/log.cpp \
../utils/rand.cpp 

OBJS += \
./utils/ThreadPool.o \
./utils/UpdateList.o \
./utils/log.o \
./utils/rand.o 

CPP_DEPS += \
./utils/ThreadPool.d \
./utils/UpdateList.d \
./utils/log.d \
./utils/rand.d 


# Each subdirectory must supply rules for building sources it contributes
utils/%.o: ../utils/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


