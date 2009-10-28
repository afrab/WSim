#!/bin/sh


source_files()
{
    LISTE=`find . -name '*.[ch]' -a ! -path './examples*' -a ! -path './test*' -a ! -path './utils/scripts/*'`
    echo ${LISTE}
}

aloc()
{
    echo "=============================================="
    echo "number of lines"
    LISTE=`source_files`
    cat $LISTE | wc -l 
}

header()
{
    echo "====================="
    echo "Wsim header structure"
    echo "====================="
    LISTE=`source_files`
#     for i in ${LISTE} ; do 
#         echo $i
#     done
    utils/scripts/cinclude2dot  --exclude './examples*|./test*|./devices*' ${LISTE} > wsim.dot 2> ch2dotlog
}


graph()
{
    LISTE=`source_files`
    graph-includes ${LISTE}
}

sloc()
{
    LIST="arch devices libconsole libelf libgdb liblogger libselect libtracer machine platforms src utils"
    echo "====================="
    echo "Sloc count           "
    echo "====================="
    sloccount $LIST
}


header
graph
sloc
aloc
