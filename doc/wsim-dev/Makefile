NAME = wsim_dev_tutorial

all: pdf html

pdf: dvi
	dvipdfm ${NAME}.dvi

dvi: ${NAME}.tex
	latex -interaction=nonstopmode $<

html: ${NAME}.tex
	latex2html --init_file=latex2html.init $<
	mkdir wsim_dev_tutorial/icon
	cp imgs/*.png wsim_dev_tutorial/icon

clean:
	rm -rf  wsim_dev_tutorial.pdf wsim_dev_tutorial.dvi wsim_dev_tutorial.html \
                wsim_dev_tutorial.aux wsim_dev_tutorial.toc wsim_dev_tutorial.log  \
                wsim_dev_tutorial                                                  \

upload: html
	cp imgs/*.png wsim_dev_tutorial		\
	rsync -ruz -e ssh --delete 		\
	wsim_dev_tutorial/			\
	afraboul@scm.gforge.inria.fr:/home/groups/wsim/htdocs/dev/


