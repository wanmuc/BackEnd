TARGET =
PREFIX = /usr/local/
RPATH = $(PREFIX)jsoncpp/libs:$(PREFIX)protobuf/lib:$(PREFIX)snappy/lib
CFLAGS = -g -O2 -Wall -Werror -pipe -m64
CXXFLAGS = -g -O2 -Wall -Werror -pipe -m64 -std=c++11
LDFLAGS = -pthread -lprotobuf -L/usr/local/protobuf/lib\
  -ljson -L/usr/local/jsoncpp/libs -lsnappy -L/usr/local/snappy/lib\
  -lrt -Wl,-rpath=$(RPATH)
INCFLAGS = -I./ -I../../common -I../../protocol\
  -I/usr/local/protobuf/include -I/usr/local/jsoncpp/include\
  -I/usr/local/snappy/include
SRCDIRS = . ./handler ./proto ../../core ../../common ../../protocol
ALONE_SOURCES =
CC = gcc
CXX = g++
SRCEXTS = .c .C .cc .cpp .CPP .c++ .cxx .cp
HDREXTS = .h .H .hh .hpp .HPP .h++ .hxx .hp
ifeq ($(TARGET),)
	TARGET = $(shell basename $(CURDIR))
	ifeq ($(TARGET),)
		TARGET = a.out
	endif
endif
ifeq ($(SRCDIRS),)
	SRCDIRS = .
endif
SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
SOURCES += $(ALONE_SOURCES)
HEADERS = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))
SRC_CXX = $(filter-out %.c,$(SOURCES))
OBJS = $(addsuffix .o, $(basename $(SOURCES)))
COMPILE.c   = $(CC)  $(CFLAGS)   $(INCFLAGS) -c
COMPILE.cxx = $(CXX) $(CXXFLAGS) $(INCFLAGS) -c
LINK.c      = $(CC)  $(CFLAGS)
LINK.cxx    = $(CXX) $(CXXFLAGS)
.PHONY: all objs clean help debug
all: $(TARGET)
objs: $(OBJS)
%.o:%.c
	$(COMPILE.c) $< -o $@
%.o:%.C
	$(COMPILE.cxx) $< -o $@
%.o:%.cc
	$(COMPILE.cxx) $< -o $@
%.o:%.cpp
	$(COMPILE.cxx) $< -o $@
%.o:%.CPP
	$(COMPILE.cxx) $< -o $@
%.o:%.c++
	$(COMPILE.cxx) $< -o $@
%.o:%.cp
	$(COMPILE.cxx) $< -o $@
%.o:%.cxx
	$(COMPILE.cxx) $< -o $@
$(TARGET): $(OBJS)
ifeq ($(SRC_CXX),)              # c程序
	$(LINK.c)   $(OBJS) -o $@ $(LDFLAGS)
	@echo Type $@ to execute the program.
else                            # c++程序
	$(LINK.cxx) $(OBJS) -o $@ $(LDFLAGS)
	@echo Type $@ to execute the program.
endif
clean:
	rm $(OBJS) $(TARGET)
help:
	@echo '通用makefile用于编译c/c++程序 版本号1.0'
	@echo
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       (等于直接执行make) 编译并连接'
	@echo '  objs      只编译不连接'
	@echo '  clean     清除目标文件和可执行文件'
	@echo '  debug     显示变量，用于调试'
	@echo '  help      显示帮助信息'
	@echo
debug:
	@echo 'TARGET       :' 	$(TARGET)
	@echo 'SRCDIRS      :'	$(SRCDIRS)
	@echo 'SOURCES      :'	$(SOURCES)
	@echo 'HEADERS      :'	$(HEADERS)
	@echo 'SRC_CXX      :'	$(SRC_CXX)
	@echo 'OBJS         :' 	$(OBJS)
	@echo 'COMPILE.c    :' 	$(COMPILE.c)
	@echo 'COMPILE.cxx  :' 	$(COMPILE.cxx)
	@echo 'LINK.c       :' 	$(LINK.c)
	@echo 'LINK.cxx     :' 	$(LINK.cxx)
