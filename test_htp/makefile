#common define
MAKE=make
CXX:=g++ -Wall
EXTRAFLAGS=-g -fpermissive -D__LINUX__
#EXTRAFLAGS=-g -fpermissive -DRECORD_LOG -D__LINUX__
INCLUDES=-I../../../
CXXFLAGS=$(EXTRAFLAGS) $(INCLUDES) 
LIB_DIR=../libs/
ALL_LIBS=-lhpf -lbytebuffer -llog4cplus -levent

OBJ_PATH:=./../objs/
SRC_PATH:=./

SRC:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp, %.o, $(SRC))
OBJS:=$(addprefix $(OBJ_PATH), $(OBJS))
DEPENDS:=$(patsubst %.cpp, %.d, $(SRC))
DEPENDS:=$(addprefix $(OBJ_PATH), $(DEPENDS))

BIN=client_demo server_demo timeout_demo
all : $(BIN)

include $(DEPENDS)


clean : 
	rm -f $(BIN) $(DEPENDS)

client_demo : ../objs/client_demo.o 
	$(CXX) $(CXXFLAGS) -o $@ $^  -L$(LIB_DIR) $(ALL_LIBS)

server_demo : ../objs/server_demo.o
	$(CXX) $(CXXFLAGS) -o $@ $^  -L$(LIB_DIR) $(ALL_LIBS)

timeout_demo : ../objs/timeout_demo.o
	$(CXX) $(CXXFLAGS) -o $@ $^  -L$(LIB_DIR) $(ALL_LIBS)
	
$(OBJ_PATH)%.o : $(SRC_PATH)%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJ_PATH)%.d : $(SRC_PATH)%.cpp
	set -e;\
	rm -f $@;\
	gcc $(INCLUDES) -MM $< > $@.$$$$;\
	sed 's,\($*\)\.o[: ],$(OBJ_PATH)\1\.o $@: ,g' < $@.$$$$ > $@;\
	rm -f $@.$$$$;
