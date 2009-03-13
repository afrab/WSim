#! /bin/sh

# ############################################################
# ############################################################

## misc
DATE=`/bin/date "+%y%m%d-%kh%M" | tr ' ' 0`

cd()
{
    if ! builtin cd $1
    then    echo "Failed to cd $1 !!!" >&2
            exit 1
    fi
}

# ############################################################
# ############################################################

LOG_LINE()
{
    echo '===' '=================================================='
    echo '===' $1
    echo '===' '=================================================='
}

# ############################################################
# ############################################################

do_compile()
{
    TARGET=$1
    CONFARG=$2
    CURDIR=`pwd`

#ECHO=echo

    LOG_LINE "start ${TARGET}"
    ${ECHO} rm -rf   ${TARGET}
    ${ECHO} mkdir -p ${TARGET}
    ${ECHO} cd       ${TARGET}
    ${ECHO} ${SRCDIR}/configure $CONFARG
    ${ECHO} make -j3
    ${ECHO} cd ${CURDIR}
}

# ############################################################
# ############################################################

do_utils()
{
    BASE="$1"
    ARGS="$2"

    LOG_LINE "tracer"
    mkdir -p ${BASE}/wtracer
    cd       ${BASE}/wtracer
    ${SRCDIR}/utils/wtracer/configure 
    make 

    LOG_LINE "console"
    mkdir -p ${BASE}/wconsole
    cd       ${BASE}/wconsole
    ${SRCDIR}/utils/console/configure ${ARGS}
    make

    LOG_LINE "wsnet1"
    mkdir -p ${BASE}/wsnet1le
    cd       ${BASE}/wsnet1le
    ${SRCDIR}/utils/wsnet1le/configure 
    make 
}

# ############################################################
# ############################################################


# ############################################################
# ############################################################
# mingw32
# ############################################################
# ############################################################

do_mingw32()
{
    export CC=i586-mingw32msvc-gcc
    export CXX=i586-mingw32msvc-g++
    export AR=i586-mingw32msvc-ar
    export RANLIB=i586-mingw32msvc-ranlib
    export CROSS_COMPILE=i586-mingw32msvc
    ARGSBASE="--host=${CROSS_COMPILE} --with-sdl-prefix=${SDL_MINGW32} --disable-ptty"

    ## WSim
    LOG_LINE "start of Mingw32 compile"
    mkdir -p ${MINGW32} ; cd ${MINGW32}

    ## 1
    ARGS="${ARGSBASE} --disable-wsnet1 --disable-wsnet2 --enable-gui --disable-debug"
    do_compile ${MINGW32}/worldsens "${ARGS}"
    ## 2
    ARGS="${ARGSBASE} --disable-wsnet1 --disable-wsnet2 --enable-gui --enable-debug"
    do_compile ${MINGW32}/worldsens-debug "${ARGS}"

    ## Wtracer
    LOG_LINE "tracer"
    mkdir -p ${MINGW32}/wtracer
    cd       ${MINGW32}/wtracer
    ${SRCDIR}/utils/tracer/configure --host=${CROSS_COMPILE}
    make 

    LOG_LINE "enf of Mingw32 cvs test"
    unset CC
    unset CXX
    unset AR
    unset RANLIB
    unset CROSS_COMPILE
}

# ############################################################
# ############################################################
# Linux
# ############################################################
# ############################################################

do_linux_ii()
{
    NAME="$1"                   # linux32 / linux64
    BASE="$2"                   # base arguments (mainly SDL)
    export ETRACELIBNAME="$3"   # etrace lib
    export CC="$4"
    export CFLAGS="$5"
    export LDFLAGS="$6"
    LOG_LINE "go ${NAME}"
    mkdir -p ${NAME} ; cd ${NAME}
    ## 1
    ARGS="${BASE} --enable-wsnet1 --disable-wsnet2 --enable-gui --enable-ptty --disable-debug"
    do_compile ${NAME}/worldsens "${ARGS}"
    ## 2
    ARGS="${BASE} --enable-wsnet1 --disable-wsnet2 --enable-gui --enable-ptty --enable-debug"
    do_compile ${NAME}/worldsens-debug "${ARGS}"
    ## 3
    ARGS="${BASE} --enable-wsnet1 --disable-wsnet2 --enable-gui --enable-ptty --enable-debug --enable-etrace"
    do_compile ${NAME}/worldsens-etrace "${ARGS}"
    ## utils
    do_utils ${NAME} "${BASE}"
    unset CC ; unset CFLAGS ; unset LDFLAGS ; unset ETRACELIBNAME
}

do_linux32()
{
    ARGS_BASE=""
    if [ "x${SDL_LINUX32}" != "x" ] ; then 
	ARGS_BASE="${ARGS_BASE} --with-sdl-prefix=${SDL_LINUX32}"
    fi
    if [ "x${ZLIB_LINUX32}" != "x" ] ; then 
	ARGS_BASE="${ARGS_BASE} --with-zlib-prefix=${ZLIB_LINUX32}"
    fi
    do_linux_ii ${LINUX32} "${ARGS_BASE}" "${ETRACE_LINUX32}" gcc -m32 -m32
}

do_linux64()
{
    ARGS_BASE=""
    if [ "x${SDL_LINUX64}" != "x" ] ; then 
	ARGS_BASE="${ARGS_BASE} --with-sdl-prefix=${SDL_LINUX64}"
    fi
    if [ "x${ZLIB_LINUX64}" != "x" ] ; then 
	ARGS_BASE="${ARGS_BASE} --with-zlib-prefix=${ZLIB_LINUX64}"
    fi
    do_linux_ii ${LINUX64} "${ARGS_BASE}" "${ETRACE_LINUX64}" gcc -m64 -m64
}


# ############################################################
# ############################################################
# MacOSX
# ############################################################
# ############################################################

do_macosx()
{

    DSTDIR=compil
    SOURCE=sources.tar.gz
    SCRIPT=buildosx.sh
    ARCHIVE=osx-build-${DATE}.tar.gz

    LOG_LINE " Clean on target host ${OSX}"
    ssh ${USER}@${OSX} rm -rf ${DSTDIR}
    ssh ${USER}@${OSX} mkdir ${DSTDIR}

    LOG_LINE " Sources preparation"
    tar zcf ${SOURCE} config.macosx.mk src

    LOG_LINE " COPY files to osx"
    scp ${SOURCE} ${USER}@${OSX}:${DSTDIR}
    scp ${SCRIPT} ${USER}@${OSX}:
    rm  ${SOURCE}

    LOG_LINE " BUILD"
    ssh ${USER}@${OSX} ./${SCRIPT}

    LOG_LINE " BACKUP"
    scp ${USER}@${OSX}:osx-build.tar.gz ${ARCHIVE}
    #    ssh ${USER}@${OSX} rm -rf ${DSTDIR}

    rm -rf ${DSTDIR}
    tar zxf ${ARCHIVE}
}

# ############################################################
# ############################################################
