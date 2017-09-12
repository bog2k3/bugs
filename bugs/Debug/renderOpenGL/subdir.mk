################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../renderOpenGL/Camera.cpp \
../renderOpenGL/GLText.cpp \
../renderOpenGL/Mesh.cpp \
../renderOpenGL/MeshRenderer.cpp \
../renderOpenGL/Renderer.cpp \
../renderOpenGL/Shape2D.cpp \
../renderOpenGL/Shape3D.cpp \
../renderOpenGL/TextureLoader.cpp \
../renderOpenGL/Viewport.cpp \
../renderOpenGL/ViewportCoord.cpp \
../renderOpenGL/glToolkit.cpp \
../renderOpenGL/shader.cpp 

OBJS += \
./renderOpenGL/Camera.o \
./renderOpenGL/GLText.o \
./renderOpenGL/Mesh.o \
./renderOpenGL/MeshRenderer.o \
./renderOpenGL/Renderer.o \
./renderOpenGL/Shape2D.o \
./renderOpenGL/Shape3D.o \
./renderOpenGL/TextureLoader.o \
./renderOpenGL/Viewport.o \
./renderOpenGL/ViewportCoord.o \
./renderOpenGL/glToolkit.o \
./renderOpenGL/shader.o 

CPP_DEPS += \
./renderOpenGL/Camera.d \
./renderOpenGL/GLText.d \
./renderOpenGL/Mesh.d \
./renderOpenGL/MeshRenderer.d \
./renderOpenGL/Renderer.d \
./renderOpenGL/Shape2D.d \
./renderOpenGL/Shape3D.d \
./renderOpenGL/TextureLoader.d \
./renderOpenGL/Viewport.d \
./renderOpenGL/ViewportCoord.d \
./renderOpenGL/glToolkit.d \
./renderOpenGL/shader.d 


# Each subdirectory must supply rules for building sources it contributes
renderOpenGL/%.o: ../renderOpenGL/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DDEBUG -I../3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


