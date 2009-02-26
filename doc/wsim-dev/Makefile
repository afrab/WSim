NAME = wsim_dev_tutorial

all: pdf html

pdf: dvi
	dvipdf ${NAME}.dvi

dvi: ${NAME}.tex
	latex -interaction=nonstopmode $<

html: ${NAME}.tex
	latex2html --init_file=latex2html.init $<

clean:
	rm -rf  wsim_dev_tutorial.pdf wsim_dev_tutorial.dvi wsim_dev_tutorial.html \
                wsim_dev_tutorial.aux wsim_dev_tutorial.toc wsim_dev_tutorial.log  \
                wsim_dev_tutorial                                                  \

		
