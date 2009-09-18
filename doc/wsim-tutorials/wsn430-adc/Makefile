HTMLSTYLE=htmlconfig.xsl
#PDFSTYLE=pdfconfig.xsl
#PDFSTYLE=/usr/share/sgml/docbook/xsl-stylesheets/fo/docbook.xsl
PDFSTYLE=/usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl
HTMLOPTION=xhtml
PDFOPTION=-pdf
XMLOPTION=-xml
XSLOPTION=-xsl

%.html: %.xml
	./xmltohtml --stringparam encoding=utf-8 $(HTMLOPTION) -m $(HTMLSTYLE) $<

%.pdf: %.xml
	fop $(XMLOPTION) $< $(XSLOPTION) $(PDFSTYLE) $(PDFOPTION) wsim.pdf

html: wsim.html
	rm -rf htdocs
	mkdir htdocs
	mv *.html 		htdocs
	cp -r navig		htdocs
	cp -r figures		htdocs 
	cp -r css 		htdocs
	cp -r files 		htdocs

upload: html 
	rsync -ruz -e ssh --delete  htdocs/ \
	afraboul@scm.gforge.inria.fr:/home/groups/wsim/htdocs/tutorials/msp430-adc
#	scp wsim.pdf afraboul@scm.gforge.inria.fr:/home/groups/wsim/htdocs/tutorials/

pdf: wsim.pdf

all: html pdf

clean:
	rm -rf htdocs *.html *.pdf *.aux *.dvi *.fo *.log *.out

