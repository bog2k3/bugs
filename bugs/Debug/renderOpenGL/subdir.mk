################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../renderOpenGL/Camera.cpp \
../renderOpenGL/Rectangle.cpp \
../renderOpenGL/Renderer.cpp \
../renderOpenGL/Viewport.cpp \
../renderOpenGL/glToolkit.cpp \
../renderOpenGL/shader.cpp 

OBJS += \
./renderOpenGL/Camera.o \
./renderOpenGL/Rectangle.o \
./renderOpenGL/Renderer.o \
./renderOpenGL/Viewport.o \
./renderOpenGL/glToolkit.o \
./renderOpenGL/shader.o 

CPP_DEPS += \
./renderOpenGL/Camera.d \
./renderOpenGL/Rectangle.d \
./renderOpenGL/Renderer.d \
./renderOpenGL/Viewport.d \
./renderOpenGL/glToolkit.d \
./renderOpenGL/shader.d 


# Each subdirectory must supply rules for building sources it contributes
renderOpenGL/%.o: ../renderOpenGL/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


