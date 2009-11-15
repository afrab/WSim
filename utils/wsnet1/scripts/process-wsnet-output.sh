#!/bin/bash

# This script parse the wsnet output log (sent to standard input) and displays
# the packets sent. Only meant to be used with standard 802.15.4 frames as it
# guesses the data size from the size field of the MAC header
# Author: Michael
# Hauspie (Michael.Hauspie@lifl.fr) Date: 2009/07/01

#set -x

function convert_byte()
{
    echo `perl -e "print(hex(\"0x$1\")-11)"`
}

PREAMBLE_SIZE=5
HEADER_SIZE=10
FOOTER_SIZE=2

PROCESSING="preamble"

COUNTER=0

ID_FILTER=$1

if test "x$ID_FILTER" = "x-h"
then
    cat 1>&2 <<EOF
Usage: cat yourwsnetlog | $0 [nodeid]

The nodeid param allows to filter only the packets sent by node nodeid. If not
present, all the packets are traced

EOF
elif test "x$ID_FILTER" != "x"
then
    ID_FILTER="grep ip:$ID_FILTER"
else
    ID_FILTER="tee /dev/null"
fi

grep TX | $ID_FILTER | cut -d: -f4,6 | sed -e 's/,size[:]*/ /g' | while read ip byte
do
    case $PROCESSING in
        "preamble")
            if test $COUNTER -eq 0
            then
                echo "############### Packet from $ip ###############"
                echo -n "preamble: "
            fi
            if test $COUNTER -lt $PREAMBLE_SIZE
            then
                COUNTER=`expr $COUNTER + 1`
                echo -n "$byte "
            else
                echo ""
                echo -n "mac header: $byte "
                DATA_SIZE=`convert_byte $byte`
                COUNTER=1
                PROCESSING="header"
            fi
            ;;
        "header")
            if test $COUNTER -lt $HEADER_SIZE
            then
                COUNTER=`expr $COUNTER + 1`
                echo -n "$byte "
            else
                echo ""
                echo -n "data: $byte "
                COUNTER=1
                PROCESSING="data"
            fi
            ;;
        "data")
            if test $COUNTER -lt $DATA_SIZE
            then
                COUNTER=`expr $COUNTER + 1`
                echo -n "$byte "
            else
                echo ""
                echo -n "footer: $byte "
                COUNTER=1
                PROCESSING="footer"
            fi
            ;;
        "footer")
            if test $COUNTER -lt $FOOTER_SIZE
            then
                COUNTER=`expr $COUNTER + 1`
                echo -n "$byte "
            else
                echo ""
                echo "############### Packet from $ip ###############"
                echo -n "preamble: $byte "
                COUNTER=1
                PROCESSING="preamble"
            fi
            ;;
    esac
done
