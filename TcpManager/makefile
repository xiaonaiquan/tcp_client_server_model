######################################################################
# Build options
CPP = g++
CPPFLAGS = -pthread -lboost_system
LINKFLAGS = -pthread -lboost_system

######################################################################
# Target
DIR = out
OBJS = $(DIR)/tcptrans.cpp.o
TARGET = $(DIR)/tcptrans.out
INCLUDE = /usr/include/boost

######################################################################
# Build
all : $(DIR) $(BASE) $(INCLUDE) $(TARGET)

$(DIR):
	 mkdir $(DIR)
 
$(TARGET) : $(OBJS)
	$(CPP) -o $@ $^ $(LINKFLAGS)
 
$(DIR)/tcptrans.cpp.o : tcptrans.cpp tcptrans.h def.h
	$(CPP) $(CPPFLAGS) -c -o $@ $<
 
# $(DIR)/1.cpp.o : 1.cpp 1.h 0.h
#  $(CPP) $(CPPFLAGS) -c -o $@ $<
 
# $(DIR)/2.cpp.o : 2.cpp 2.h 0.h
#  $(CPP) $(CPPFLAGS) -c -o $@ $<
 
.PHONEY:clean

clean:
	rm -rf out
