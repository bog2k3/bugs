################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Prototype.cpp \
../main.cpp \
../memdebug.cpp 

OBJS += \
./Prototype.o \
./main.o \
./memdebug.o 

CPP_DEPS += \
./Prototype.d \
./main.d \
./memdebug.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DDEBUG -I../3rdparty/easyunit -I/home/bog/work/boglfw/build/dist/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


