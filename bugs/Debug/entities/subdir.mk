################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../entities/Box.cpp \
../entities/Bug.cpp \
../entities/BugGenome.cpp \
../entities/CameraController.cpp \
../entities/Entity.cpp \
../entities/Gamete.cpp \
../entities/PathController.cpp \
../entities/Wall.cpp 

OBJS += \
./entities/Box.o \
./entities/Bug.o \
./entities/BugGenome.o \
./entities/CameraController.o \
./entities/Entity.o \
./entities/Gamete.o \
./entities/PathController.o \
./entities/Wall.o 

CPP_DEPS += \
./entities/Box.d \
./entities/Bug.d \
./entities/BugGenome.d \
./entities/CameraController.d \
./entities/Entity.d \
./entities/Gamete.d \
./entities/PathController.d \
./entities/Wall.d 


# Each subdirectory must supply rules for building sources it contributes
entities/%.o: ../entities/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DDEBUG -I../3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


