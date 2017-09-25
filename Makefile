PROJECT_DIR=.
#GLOG_PATH=-I $(PROJECT_DIR)/glog/include/
INC_PATH=-I $(PROJECT_DIR)/include 
LIB_PATH=-L $(PROJECT_DIR)/lib
#EASYLOG_SOURCE= $(PROJECT_DIR)/lib/easylogging++.cc
LIBS=-lzmq -lpthread 
FLAGS=-std=c++11
CPP=g++

OBJECTS=log_to_file.o ini.o ServerConfig.o easylogging++.o

TARGET=log_to_file

all:$(TARGET)

$(TARGET):$(OBJECTS)
	$(CPP) -o $@ $(FLAGS) $(LIB_PATH) $(LIBS) $(INC_PATH) $(OBJECTS)  

include $(OBJECTS:.o=.d)

%.o: %.cpp 
	$(CPP) -c $(FLAGS) $(INC_PATH) $< -o $@

%.d: %.cpp
	set -e; rm -f $@; \
	$(CPP) -MM $(FLAGS) $(INC_PATH) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
.PHONY:clean
clean:
	-rm -f $(OBJECTS) *.d *.d.* *~
