################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../body-parts/BodyPart.cpp \
../body-parts/Bone.cpp \
../body-parts/EggLayer.cpp \
../body-parts/Gripper.cpp \
../body-parts/Joint.cpp \
../body-parts/Mouth.cpp \
../body-parts/Muscle.cpp \
../body-parts/Torso.cpp \
../body-parts/ZygoteShell.cpp 

OBJS += \
./body-parts/BodyPart.o \
./body-parts/Bone.o \
./body-parts/EggLayer.o \
./body-parts/Gripper.o \
./body-parts/Joint.o \
./body-parts/Mouth.o \
./body-parts/Muscle.o \
./body-parts/Torso.o \
./body-parts/ZygoteShell.o 

CPP_DEPS += \
./body-parts/BodyPart.d \
./body-parts/Bone.d \
./body-parts/EggLayer.d \
./body-parts/Gripper.d \
./body-parts/Joint.d \
./body-parts/Mouth.d \
./body-parts/Muscle.d \
./body-parts/Torso.d \
./body-parts/ZygoteShell.d 


# Each subdirectory must supply rules for building sources it contributes
body-parts/%.o: ../body-parts/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -I/home/bog/work/bugs/3rdparty/easyunit -O2 -g3 -p -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


