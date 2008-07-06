#! /bin/sh


LISTE=`find . -maxdepth 1 -type d -a ! -path ./CVS*  -a ! -path .`

build_clean()
{
    for i in ${LISTE} ; do 
	echo == $i
	(cd $i ; make clean ) > log
    done
}

build()
{
    for i in ${LISTE} ; do 
	echo == $i
	(cd $i ; make clean ; make) > log
    done
}

build
