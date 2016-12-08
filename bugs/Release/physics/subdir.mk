################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../physics/PhysContactListener.cpp \
../physics/PhysDestroyListener.cpp \
../physics/PhysicsBody.cpp \
../physics/PhysicsDebugDraw.cpp 

OBJS += \
./physics/PhysContactListener.o \
./physics/PhysDestroyListener.o \
./physics/PhysicsBody.o \
./physics/PhysicsDebugDraw.o 

CPP_DEPS += \
./physics/PhysContactListener.d \
./physics/PhysDestroyListener.d \
./physics/PhysicsBody.d \
./physics/PhysicsDebugDraw.d 


# Each subdirectory must supply rules for building sources it contributes
physics/%.o: ../physics/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -I../3rdparty/easyunit -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


