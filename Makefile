VERSION   = 0.0.0
CXX       = g++ -std=c++0x
CXXFLAGS += -g -Wall -DVERSION=\"$(VERSION)\" -Ispdr
prefix   ?= /usr/local/

ifeq ($(MSYSTEM), MINGW32)
  EXEEXT  = .exe  
  LIBEXT  = .dll
else
  EXEEXT  =
  LIBEXT  = .so  
endif

spdr_headers = $(wildcard spdr/*.h)
spdr_src     = $(wildcard spdr/*.cpp)
spdr_libs    = -lsigc-2.0 -lc9y -lwsock32
test_src     = $(wildcard test/*.cpp)
test_libs    = -lUnitTest++ -lsigc-2.0 -lc9y
dist_files   = $(spdr_headers) $(spdr_src) $(test_src) Makefile README.md

.PHONY: all clean check install uninstall dist
.SUFFIXES: .o .cpp

all: spdr$(LIBEXT)

spdr$(LIBEXT): $(patsubst %.cpp, %.o, $(spdr_src))
	$(CXX) -shared -fPIC $(CXXFLAGS) $(LDFLAGS) $^ $(spdr_libs) -Wl,--out-implib=$(patsubst %$(LIBEXT),lib%.a, $@) -o $@

clean:
	rm -f */*.o */*.d *$(LIBEXT) *.a test$(EXEEXT)

check: test$(EXEEXT)
	./test$(EXEEXT)

test$(EXEEXT): spdr$(LIBEXT) $(patsubst %.cpp, %.o, $(test_src))
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(test_libs) -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -MD -c $< -o $(patsubst %.cpp, %.o, $<)

install: all
	mkdir -p $(prefix)include/spdr/
	cp spdr/*.h $(prefix)include/spdr
	mkdir -p $(prefix)bin
	cp spdr$(LIBEXT) $(prefix)bin/
	mkdir -p $(prefix)lib
	cp libspdr.a $(prefix)lib/

uninstall:
	rm -rf $(prefix)include/spdr
	rm -rf $(prefix)bin/spdr$(LIBEXT)
	rm -rf $(prefix)lib/libspdr.a

dist:
	mkdir spdr-$(VERSION)
	cp --parents $(dist_files) spdr-$(VERSION)/
	tar -czvf spdr-$(VERSION).tar.gz spdr-$(VERSION)
	rm -rf spdr-$(VERSION)

ifneq "$(MAKECMDGOALS)" "clean"
deps  = $(patsubst %.cpp, %.d, $(spdr_src))
deps += $(patsubst %.cpp, %.d, $(test_src))
-include $(deps)
endif
