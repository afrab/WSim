TEXI=wsim-user.texi 			\
	periph/wsim-7seg.texi 		\
	periph/wsim-cc1100.texi		\
	periph/wsim-ds2411.texi		\
	periph/wsim-gdm1602a.texi	\
	periph/wsim-led.texi		\
	periph/wsim-m25p80.texi		\
	periph/wsim-ptty.texi		

MAINTEXI=wsim-user.texi

html: ${TEXI}
	 texi2html --init-file=texi2html.init ${MAINTEXI}

dvi: ${TEXI}
	texi2dvi ${MAINTEXI}

%.pdf: %.dvi
	dvipdf $<

clean:
	rm -f wsim-user.aux wsim-user.cp wsim-user.dvi wsim-user.fn		\
		wsim-user.html wsim-user.info wsim-user.ky wsim-user.log 	\
		wsim-user.pdf wsim-user.pg wsim-user.toc			\
		wsim-user.tp wsim-user.vr
