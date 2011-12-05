lib_LTLIBRARIES = libxtestingenvironment.la libxtestingenvironment_main.la

libxtestingenvironment_la_SOURCES = \
	src/environment.cpp

libxtestingenvironment_main_la_SOURCES = \
	src/environment.cpp \
	src/main.cpp

library_includedir=$(includedir)/xtestingenvironment-1.0/xtestingenvironment
library_include_HEADERS = src/environment.h

library_datadir=$(datadir)/xtestingenvironment-1.0
library_data_DATA=conf/dummy.conf

libxtestingenvironment_la_LDFLAGS = $(X11_LIBS) $(GTEST_LDFLAGS)
libxtestingenvironment_main_la_LDFLAGS = $(X11_LIBS) $(GTEST_LDFLAGS)

ACLOCAL_AMFLAGS = -I m4

.PHONY: ChangeLog INSTALL
INSTALL:
	$(INSTALL_CMD)

doc-%:
	$(MAKE) -C doc $@

dist-hook: ChangeLog INSTALL
