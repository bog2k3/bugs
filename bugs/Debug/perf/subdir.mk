################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../perf/callGraph.cpp \
../perf/frameCapture.cpp \
../perf/results.cpp 

OBJS += \
./perf/callGraph.o \
./perf/frameCapture.o \
./perf/results.o 

CPP_DEPS += \
./perf/callGraph.d \
./perf/frameCapture.d \
./perf/results.d 


# Each subdirectory must supply rules for building sources it contributes
perf/%.o: ../perf/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DGLM_FORCE_RADIANS -DDEBUG -I../3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


