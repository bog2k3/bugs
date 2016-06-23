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
entities/Bug.o: /mnt/docs/Work/bugs/bugs/entities/Bug.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

entities/BugGenome.o: /mnt/docs/Work/bugs/bugs/entities/BugGenome.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

entities/Entity.o: /mnt/docs/Work/bugs/bugs/entities/Entity.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

entities/Gamete.o: /mnt/docs/Work/bugs/bugs/entities/Gamete.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

entities/Wall.o: /mnt/docs/Work/bugs/bugs/entities/Wall.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


