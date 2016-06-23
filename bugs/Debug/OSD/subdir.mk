################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../OSD/EntityLabeler.cpp \
../OSD/Label.cpp \
../OSD/ScaleDisplay.cpp \
../OSD/SignalViewer.cpp 

OBJS += \
./OSD/EntityLabeler.o \
./OSD/Label.o \
./OSD/ScaleDisplay.o \
./OSD/SignalViewer.o 

CPP_DEPS += \
./OSD/EntityLabeler.d \
./OSD/Label.d \
./OSD/ScaleDisplay.d \
./OSD/SignalViewer.d 


# Each subdirectory must supply rules for building sources it contributes
OSD/EntityLabeler.o: /mnt/docs/Work/bugs/bugs/OSD/EntityLabeler.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

OSD/Label.o: /mnt/docs/Work/bugs/bugs/OSD/Label.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

OSD/ScaleDisplay.o: /mnt/docs/Work/bugs/bugs/OSD/ScaleDisplay.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

OSD/SignalViewer.o: /mnt/docs/Work/bugs/bugs/OSD/SignalViewer.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


