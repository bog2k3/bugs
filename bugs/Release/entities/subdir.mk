################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../entities/Bug.cpp \
../entities/BugGenome.cpp \
../entities/Entity.cpp \
../entities/Gamete.cpp \
../entities/Wall.cpp 

OBJS += \
./entities/Bug.o \
./entities/BugGenome.o \
./entities/Entity.o \
./entities/Gamete.o \
./entities/Wall.o 

CPP_DEPS += \
./entities/Bug.d \
./entities/BugGenome.d \
./entities/Entity.d \
./entities/Gamete.d \
./entities/Wall.d 


# Each subdirectory must supply rules for building sources it contributes
entities/%.o: ../entities/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -I../3rdparty/easyunit -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


