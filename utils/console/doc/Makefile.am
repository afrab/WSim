MAINTAINERCLEANFILES    = Makefile.in
info_TEXINFOS           = wconsole.texi

SUFFIXES                = .html

html_docs               = wconsole.html

.texi.html:
	$(MAKEINFO) --html $<

.PHONY: html
html: wconsole.texi $(html_docs)
