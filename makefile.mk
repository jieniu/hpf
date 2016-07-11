INCLUDES=-I/usr/local/include/libevent-2.0.16/include/

#lib define
#LIB_DIR=-Lbytebuffer
LIB_SSL=-lssl

#ALL_LIBS=-lbytebuffer -llog4cplus -levent 
ALL_LIBS=-llog4cplus -levent 

#common define
MAKE=make
CXX:=g++ -Wall
MV=/bin/mv
EXTRAFLAGS=-g -DRECORD_LOG -D__LINUX__
#EXTRAFLAGS=-g -D__LINUX__
CXXFLAGS=$(EXTRAFLAGS) $(INCLUDES) 

RM=rm -f
AR=ar rc


OBJ_PATH:=objs/
SRC_PATH:=./

SRC:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp, %.o, $(SRC))
OBJS:=$(addprefix $(OBJ_PATH), $(OBJS))
DEPENDS:=$(patsubst %.cpp, %.d, $(SRC))
DEPENDS:=$(addprefix $(OBJ_PATH), $(DEPENDS))

include $(DEPENDS)

# High Performance Framework
BIN=libs/libhpf.a

all: $(BIN) 

$(BIN):$(OBJS) 
	ar cr $@ $^
#	$(CXX) $(CXXFLAGS) -o $@ $^ -Lbytebuffer -lbytebuffer $(LIB_DIR) $(ALL_LIBS)
clean:
	$(RM)  $(OBJS) $(BIN) $(BIN).tar.bz2 $(BIN)_bin $(DEPENDS)


$(OBJ_PATH)%.o : $(SRC_PATH)%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJ_PATH)%.d : $(SRC_PATH)%.cpp
	set -e;\
	$(RM) $@;\
	gcc $(INCLUDES) -MM $< > $@.$$$$;\
	sed 's,\($*\)\.o[: ],$(OBJ_PATH)\1\.o $@: ,g' < $@.$$$$ > $@;\
	$(RM) $@.$$$$;
