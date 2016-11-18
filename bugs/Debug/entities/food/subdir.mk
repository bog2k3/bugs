################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../entities/food/FoodChunk.cpp \
../entities/food/FoodDispenser.cpp 

OBJS += \
./entities/food/FoodChunk.o \
./entities/food/FoodDispenser.o 

CPP_DEPS += \
./entities/food/FoodChunk.d \
./entities/food/FoodDispenser.d 


# Each subdirectory must supply rules for building sources it contributes
entities/food/%.o: ../entities/food/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DDEBUG -I../3rdparty/easyunit -O0 -g3 -p -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


