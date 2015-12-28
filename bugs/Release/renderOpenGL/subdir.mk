################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../renderOpenGL/Camera.cpp \
../renderOpenGL/GLText.cpp \
../renderOpenGL/Renderer.cpp \
../renderOpenGL/Shape2D.cpp \
../renderOpenGL/TextureLoader.cpp \
../renderOpenGL/Viewport.cpp \
../renderOpenGL/glToolkit.cpp \
../renderOpenGL/shader.cpp 

OBJS += \
./renderOpenGL/Camera.o \
./renderOpenGL/GLText.o \
./renderOpenGL/Renderer.o \
./renderOpenGL/Shape2D.o \
./renderOpenGL/TextureLoader.o \
./renderOpenGL/Viewport.o \
./renderOpenGL/glToolkit.o \
./renderOpenGL/shader.o 

CPP_DEPS += \
./renderOpenGL/Camera.d \
./renderOpenGL/GLText.d \
./renderOpenGL/Renderer.d \
./renderOpenGL/Shape2D.d \
./renderOpenGL/TextureLoader.d \
./renderOpenGL/Viewport.d \
./renderOpenGL/glToolkit.d \
./renderOpenGL/shader.d 


# Each subdirectory must supply rules for building sources it contributes
renderOpenGL/%.o: ../renderOpenGL/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -I"/mnt/docs/Work/bugs/bugs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


