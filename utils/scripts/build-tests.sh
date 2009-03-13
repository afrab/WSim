#! /bin/sh
# set -x

# ############################################################
# ############################################################

do_grep()
{
    LOG_LINE " Compiler errors"
    grep '\*\*\*' $1
    grep -in -A3 -B3  'error:'   $1
    LOG_LINE " Compiler warnings"
    grep -in -A3 -B3  'warning:' $1
}

do_errors()
{
    LOG=$1
    REF=$2

    LOG_LINE "Compile errors and warnings for $LOG"
    diff $LOG $REF
}

# ############################################################
# ############################################################

do_tests()
{
    case $1 in
	linux32)
	    echo "tests linux32"
	    ;;
	linux64)
	    echo "tests linux64"
	    ;;
	mingw32)
    	    echo "tests mingw32"
	    ;;
	macosx)
	    echo "tests MacOSX"
	    ;;
    esac
}

# ############################################################
# ############################################################
# ############################################################
