
VERSION   = 0.1.0
CXX       = g++ -std=c++0x
prefix   ?= /usr/local/

C9Y_CFLAGS := $(shell pkg-config c9y --cflags)
C9Y_LIBS   := $(shell pkg-config c9y --libs)

ifeq ($(MSYSTEM), MINGW32)
  EXEEXT  = .exe  
  LIBEXT  = .dll
else
  EXEEXT  =
  LIBEXT  = .so  
endif

CXXFLAGS        += -g -Wall -DVERSION=\"$(VERSION)\" -I. $(C9Y_CFLAGS)

inst_HEADERS   := spdr/spdr.h spdr/Node.h spdr/Peer.h spdr/pack.h
noinst_HEADERS := 
spdr_SOURCES   := spdr/Node.cpp spdr/Peer.cpp spdr/pack.cpp
spdr_LDADD     := $(C9Y_LIBS)


test_SOURCES   := test/main.cpp
test_LDADD     := libspdr.a -lrtest


noinst_HEADERS      += examples/chat/defines.h
char_server_SOURCES := examples/chat/server.cpp
char_server_LDADD   := 
char_client_SOURCES := examples/chat/client.cpp
char_client_LDADD   := 



extadist_FILES := Makefile README.md

.PHONY: all clean check install uninstall dist
.SUFFIXES: .o .cpp

all: spdr$(LIBEXT)

spdr$(LIBEXT): $(patsubst %.cpp, %.o, $(spdr_SOURCES))
	$(CXX) -shared -fPIC $(CXXFLAGS) $(LDFLAGS) $^ $(spdr_LDADD) -Wl,--out-implib,libspdr.a -o $@
	
libspdr.a: spdr$(LIBEXT)

clean:
	rm -f */*.o */*.d *$(LIBEXT) *.a test$(EXEEXT)

check: test$(EXEEXT)
	./test$(EXEEXT)

test$(EXEEXT): $(patsubst %.cpp, %.o, $(test_SOURCES)) libspdr.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(test_LDADD) -o $@

examples: chat-server$(EXEEXT) chat-client$(EXEEXT)
	
chat-server$(EXEEXT): $(patsubst %.cpp, %.o, $(char_server_SOURCES)) libspdr.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(char_server_LDADD) -o $@	
	
chat-client$(EXEEXT): $(patsubst %.cpp, %.o, $(char_client_SOURCES)) libspdr.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(char_client_LDADD) -o $@	
	
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -MD -c $< -o $(patsubst %.cpp, %.o, $<)

install: all
	mkdir -p $(prefix)include/spdr/
	cp $(inst_HEADERS) $(prefix)include/spdr
	mkdir -p $(prefix)bin
	cp spdr$(LIBEXT) $(prefix)bin/
	mkdir -p $(prefix)lib
	cp libspdr.a $(prefix)lib/

uninstall:
	rm -rf $(prefix)include/spdr
	rm -rf $(prefix)bin/spdr$(LIBEXT)
	rm -rf $(prefix)lib/libspdr.a

dist_FILES := $(inst_HEADERS) $(spdr_SOURCES) $(test_SOURCES) $(extadist_FILES) \
			  $(char_server_SOURCES) $(char_client_SOURCES) $(noinst_HEADERS)
	
dist:
	mkdir spdr-$(VERSION)
	cp --parents $(dist_FILES) spdr-$(VERSION)/
	tar -czvf spdr-$(VERSION).tar.gz spdr-$(VERSION)
	rm -rf spdr-$(VERSION)

ifneq "$(MAKECMDGOALS)" "clean"
deps  = $(patsubst %.cpp, %.d, $(spdr_SOURCES))
deps += $(patsubst %.cpp, %.d, $(test_SOURCES))
deps += $(patsubst %.cpp, %.d, $(char_server_SOURCES))
deps += $(patsubst %.cpp, %.d, $(char_client_SOURCES))
-include $(deps)
endif
