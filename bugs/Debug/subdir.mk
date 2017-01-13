################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Infrastructure.cpp \
../SpatialCache.cpp \
../World.cpp \
../main.cpp \
../memdebug.cpp \
../perfPrint.cpp 

OBJS += \
./Infrastructure.o \
./SpatialCache.o \
./World.o \
./main.o \
./memdebug.o \
./perfPrint.o 

CPP_DEPS += \
./Infrastructure.d \
./SpatialCache.d \
./World.d \
./main.d \
./memdebug.d \
./perfPrint.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DDEBUG -I../3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


