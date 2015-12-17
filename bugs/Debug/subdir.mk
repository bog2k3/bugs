################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PhysContactListener.cpp \
../PhysDestroyListener.cpp \
../PhysicsBody.cpp \
../PhysicsDebugDraw.cpp \
../World.cpp \
../main.cpp 

OBJS += \
./PhysContactListener.o \
./PhysDestroyListener.o \
./PhysicsBody.o \
./PhysicsDebugDraw.o \
./World.o \
./main.o 

CPP_DEPS += \
./PhysContactListener.d \
./PhysDestroyListener.d \
./PhysicsBody.d \
./PhysicsDebugDraw.d \
./World.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


