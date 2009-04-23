#!/bin/bash
# $Id: autogen.sh,v 1.1.1.1 2006/07/26 17:40:22 otaviocc Exp $

verbConf()
{
    echo "% '$*'..."
    $*
}

echo "# CC: $CC"
echo "# CXX: $CXX"
echo "# CFLAGS: $CFLAGS"
echo "# CXXFLAGS: $CXXFLAGS"
echo "# LDFLAGS: $LDFLAGS"
verbConf "libtoolize -c --force"
verbConf "aclocal"
verbConf "autoheader"
verbConf "automake -a -f --foreign"
verbConf "autoconf"
