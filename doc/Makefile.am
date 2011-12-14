##
## Makefile for the doc subdirectory of xorg-gtest
##
## Copyright (C) 2011 Canonical Ltd.
##
## Process this file with automake to produce Makefile.in.
##
## This file is part of the libgeis project.  This library is free
## software; you can redistribute it and/or modify it under the
## terms of the GNU General Public License as published by the
## Free Software Foundation; either version 3, or (at your option)
## any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this library; see the file COPYING3.  If not see
## <http://www.gnu.org/licenses/>.
##

dist_doc_DATA = \
	Doxyfile 

doc-html: doc-html-doxygen 

doc-man: doc-man-doxygen

doc-pdf: doc-pdf-doxygen

doc-html-doxygen:
	doxygen 

doc-man-doxygen:
	doxygen 

doc-pdf-doxygen:
	doxygen 

install-data-local: api/html
	$(INSTALL) -d $(DESTDIR)$(docdir) 
	cp -a api/html $(DESTDIR)$(docdir) 

clean-local:
	-rm -rf *.pdf *.html api
