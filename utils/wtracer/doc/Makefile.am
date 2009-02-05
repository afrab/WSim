MAINTAINERCLEANFILES    = Makefile.in
info_TEXINFOS           = wtracer.texi

SUFFIXES                = .html

html_docs               = wtracer.html

.texi.html:
	$(MAKEINFO) --html $<

.PHONY: html
html: wtracer.texi $(html_docs)
