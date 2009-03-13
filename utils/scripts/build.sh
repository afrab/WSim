#! /bin/sh
# set -x

unset LANG

# ############################################################
# ############################################################

## wsim repository src
SRCDIR=${HOME}/projets/worldsens/wsim
TSTDIR=${HOME}/projets/worldsens/build/0-test-src
REFDIR=${SRCDIR}/utils/scripts/tests


## target builds
LINUX32=${TSTDIR}/linux32
LINUX64=${TSTDIR}/linux64
MINGW32=${TSTDIR}/mingw32


export SDL_LINUX32=${HOME}/projets/worldsens/build/0-test-cvs/src/libs/linux32/SDL-1.2.12
export SDL_LINUX64=${HOME}/projets/worldsens/build/0-test-cvs/src/libs/linux64/SDL-1.2.12
export SDL_MINGW32=${HOME}/projets/worldsens/build/0-test-cvs/src/libs/mingw32/SDL-1.2.12

export ZLIB_LINUX32=${HOME}/projets/worldsens/build/0-test-cvs/src/libs/linux32/zlib1g
export ZLIB_LINUX64=${HOME}/projets/worldsens/build/0-test-cvs/src/libs/linux64/zlib1g
export ZLIB_MINGW32=${HOME}/projets/worldsens/build/0-test-cvs/src/libs/mingw32/zlib1g

export ETRACELIBPATH=${HOME}/projets/os_basse_conso/tools/skyeye/etracelib
export ETRACE_LINUX32=libetrace32.a
export ETRACE_LINUX64=libetrace64.a
# export ETRACE_MINGW32=libetraceMGW.a

# ############################################################
# ############################################################

source ${SRCDIR}/utils/scripts/build-helpers.sh
source ${SRCDIR}/utils/scripts/build-tests.sh

# ############################################################
# ############################################################

cd $TSTDIR

case $1 in
    linux32)
	echo "linux32"
	( do_linux32 )                                      >  log.linux32.log  2>&1
	do_errors log.linux32.log ${REFDIR}/log.linux32.ref >  log.linux32.txt  2>&1
	;;
    linux64)
	echo "linux64"
	( do_linux64 )                                      >  log.linux64.log  2>&1
	do_errors log.linux64.log ${REFDIR}/log.linux64.ref >  log.linux64.txt  2>&1
	;;
    mingw32)
	echo "mingw32"
	( do_mingw32 )                                      >  log.mingw32.log  2>&1
	do_errors log.mingw32.log ${REFDIR}/log.mingw32.ref >  log.mingw32.txt  2>&1
	;;
    macosx)
	echo "macosx"
	( do_macosx )                                       >  log.macosx.log   2>&1
	do_errors log.macosx.log  ${REFDIR}/log.mingw32.ref >  log.macosx.txt   2>&1
	;;
    all)
	echo "all"
	( do_linux32 ; do_linux64 ; do_mingw32 )            >  log.all.log      2>&1
	do_errors log.all.log ${REFDIR}/log.all.ref         >  log.all.txt      2>&1
	do_tests all
	;;
    clean)
	rm -f  *.log *.txt
	rm -rf linux32 linux64 mingw32
	;;
    *)
	echo "usage: target tests"
	echo ""
	echo "       linux32"
	echo "       linux64"
	echo "       mingw32"
	echo ""
	echo "       all"
	echo "       tests"
	echo "       clean"
	;;
esac

if [ "x$2" == "xtests" ] ; then 
    TESTS=yes
    do_tests $1
fi

# ############################################################
# ############################################################
# ############################################################
