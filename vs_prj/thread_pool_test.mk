app=test_raft_log

SOURCES=test_raft_log.cpp

OBJECTS=$(SOURCES:.cpp=.o)

all: $(app)

DEBUG=-g
INCLUDES=-I/usr/include/ -I../
OPT=-O3
DEFINES=-DBOOST_LOG_DYN_LINK
LIBS=-pthread -lboost_log -lboost_log_setup -lboost_thread -lboost_system -lgtest -lgtest_main

CXXFLAGS=-std=c++17 -MD -O3 -Wall -Wextra $(DEFINES) $(INCLUDES) $(OPT) $(DEBUG)
CXXLINKS=$(CXXFLAGS) $(LIBS)

COMPILER=clang++

$(app): %: %.o $(OBJECTS)
	$(COMPILER) $(CXXLINKS) $^ -o $@

%.o: %.cpp
	$(COMPILER) $(CXXFLAGS) -c $^

-include $(SOURCES:=.d) $(apps:=.d)

clean:
	-rm $(app) *.o *.d 2> /dev/null
