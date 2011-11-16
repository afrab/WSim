#! /bin/sh


LISTE=`find . -maxdepth 2 -type d -a ! -path ./CVS*  -a ! -path .`

build_clean()
{
    for i in ${LISTE} ; do 
	echo == $i
	(
	    cd $i  
	    if [ -f Makefile ] ; then
		make clean 
	    fi
	) > log
    done
}

build()
{
    for i in ${LISTE} ; do 
	echo == $i
	(
	    cd $i
	    if [ -f Makefile ] ; then
		make clean ; make
	    fi
	) > log
    done
}

build
