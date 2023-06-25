CFLAGS= -std=c++17 -O2
CXX ?= g++
CC ?= gcc
OBJS = task.o task_flowSize.o task_throughput.o \
	task_CE.o task_WMRE.o task_HH_ARE.o task_HH_F1.o

all: $(OBJS) objmove

task.o: task.cpp task.h
	$(CXX) $(CFLAGS) -c task.cpp 

task_flowSize.o: task_flowSize.cpp task_flowSize.h
	$(CXX) $(CFLAGS) -c task_flowSize.cpp 

task_throughput.o: task_throughput.cpp task_throughput.h
	$(CXX) $(CFLAGS) -c task_throughput.cpp 

task_CE.o: task_CE.cpp task_CE.h
	$(CXX) $(CFLAGS) -c task_CE.cpp 

task_WMRE.o: task_WMRE.cpp task_WMRE.h
	$(CXX) $(CFLAGS) -c task_WMRE.cpp 

task_HH_ARE.o: task_HH_ARE.cpp task_HH_ARE.cpp
	$(CXX) $(CFLAGS) -c task_HH_ARE.cpp

task_HH_F1.o: task_HH_F1.cpp task_HH_F1.cpp
	$(CXX) $(CFLAGS) -c task_HH_F1.cpp

objmove:
	cp $(OBJS) ../

clean:
	rm -rf $(OBJS)