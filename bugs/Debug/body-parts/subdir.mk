################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../body-parts/BodyCell.cpp \
../body-parts/BodyPart.cpp \
../body-parts/Bone.cpp \
../body-parts/Cell.cpp \
../body-parts/EggLayer.cpp \
../body-parts/FatCell.cpp \
../body-parts/Gripper.cpp \
../body-parts/JointPivot.cpp \
../body-parts/JointWeld.cpp \
../body-parts/Mouth.cpp \
../body-parts/Muscle.cpp \
../body-parts/Torso.cpp \
../body-parts/ZygoteShell.cpp 

OBJS += \
./body-parts/BodyCell.o \
./body-parts/BodyPart.o \
./body-parts/Bone.o \
./body-parts/Cell.o \
./body-parts/EggLayer.o \
./body-parts/FatCell.o \
./body-parts/Gripper.o \
./body-parts/JointPivot.o \
./body-parts/JointWeld.o \
./body-parts/Mouth.o \
./body-parts/Muscle.o \
./body-parts/Torso.o \
./body-parts/ZygoteShell.o 

CPP_DEPS += \
./body-parts/BodyCell.d \
./body-parts/BodyPart.d \
./body-parts/Bone.d \
./body-parts/Cell.d \
./body-parts/EggLayer.d \
./body-parts/FatCell.d \
./body-parts/Gripper.d \
./body-parts/JointPivot.d \
./body-parts/JointWeld.d \
./body-parts/Mouth.d \
./body-parts/Muscle.d \
./body-parts/Torso.d \
./body-parts/ZygoteShell.d 


# Each subdirectory must supply rules for building sources it contributes
body-parts/%.o: ../body-parts/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL -DDEBUG -I../3rdparty/easyunit -I/home/bog/work/boglfw/build/dist/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


