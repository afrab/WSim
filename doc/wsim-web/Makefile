HTMLX=	\
	contact.htmlx		\
	contributors.htmlx	\
	download.htmlx		\
	faq.htmlx		\
	index.htmlx		\
	installation.htmlx	\
	userdoc.htmlx		\
	license.htmlx		\
	models.htmlx		\
	overview.htmlx		\
	people.htmlx		\
	projects.htmlx		\
	publications.htmlx	\
	related.htmlx		\
	tools.htmlx		\
	tutorial.htmlx		\
	users.htmlx

HTML=$(HTMLX:.htmlx=.html)

all: ${HTML}
	rm -rf 				htdocs
	mkdir 				htdocs
	mv *.html 			htdocs
	cp -r styles 			htdocs
	cp -r figures 			htdocs
	cp -r tutorials 		htdocs

upload: all
	rsync -ruz -e ssh --delete 		\
	--exclude=log/ 				\
	--exclude=tutorials/	 		\
	htdocs/ 				\
	afraboul@scm.gforge.inria.fr:/home/groups/wsim/htdocs

%.html: htmlx/%.htmlx
	gcc -E -x c -P -C -Itemplate $< > $@

user:doc-user/user.html

doc-user/user.html: ../user/wsim-user.texi
	mkdir -p doc-user
	texi2html --init-file texi2html.conf  -I ../user ../user/wsim-user.texi

clean:
	rm -rf htdocs
