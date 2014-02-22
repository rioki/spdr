ACLOCAL_AMFLAGS           = -I m4

AM_CXXFLAGS               = -g -Wall -Ispdr

pkginclude_HEADERS        = spdr/spdr.h \
                            spdr/Node.h \
                            spdr/IpAddress.h \
                            spdr/UdpSocket.h \
                            spdr/pack.h
                            
pkgconfigdir 		      = $(libdir)/pkgconfig
pkgconfig_DATA 	          = spdr.pc

lib_LTLIBRARIES           = libspdr.la

libspdr_la_LDFLAGS        = -no-undefined -version-info 0:0:0
libspdr_la_SOURCES        = spdr/Node.cpp \
                            spdr/IpAddress.cpp \
                            spdr/UdpSocket.cpp \
                            spdr/pack.cpp

noinst_HEADERS            = examples/chat/defines.h
                            
noinst_PROGRAMS           = chat-server chat-client
                            
chat_server_LDADD         = libspdr.la
chat_server_SOURCES       = examples/chat/server.cpp

chat_client_LDADD         = libspdr.la
chat_client_SOURCES       = examples/chat/client.cpp
                     
EXTRA_DIST                = README.md
