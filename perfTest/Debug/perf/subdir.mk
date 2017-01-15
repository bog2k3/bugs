################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../perf/callGraph.cpp \
../perf/results.cpp \
../perf/threadMarker.cpp 

OBJS += \
./perf/callGraph.o \
./perf/results.o \
./perf/threadMarker.o 

CPP_DEPS += \
./perf/callGraph.d \
./perf/results.d \
./perf/threadMarker.d 


# Each subdirectory must supply rules for building sources it contributes
perf/%.o: ../perf/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++14 -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


