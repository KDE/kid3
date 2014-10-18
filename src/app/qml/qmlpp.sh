#!/bin/bash
#
# qmlpp - KISS QML and JavaScript preprocessor
#
# Copyright (C) 2013 Andre Beckedorf
#       <evilJazz _AT_ katastrophos _DOT_ net>
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version
# 2.1 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301  USA
#
# Alternatively, this file is available under the Mozilla Public
# License Version 1.1.  You may obtain a copy of the License at
# http://www.mozilla.org/MPL/

findCmd=$(which find)
sedCmd=$(which sed)
sedOSCmdExt=

isWindows=$(echo $OS | grep "Windows")
if [ "$isWindows" != "" ]; then
        findCmd="/usr/bin/find" # to override Microsoft Windows FIND command and
        sedCmd="/usr/bin/sed"   # make sure the Cygwin's find and sed are launched...
        sedOSCmdExt=-b
fi

sedPingTest=$(echo "ping" | $sedCmd -r "s/ping/pong/" 2> /dev/null)
if [ "$sedPingTest" != "pong" ]; then
    sedIsBSD=true
    sedCmdExt=-E
else
    sedIsBSD=
    sedCmdExt=-r
fi

function callSed()
{
    if [[ -n "$sedIsBSD" && -n "$processInline" ]]; then
        "$sedCmd" $sedOSCmdExt $sedCmdExt -i '' "$@" # Braindead BSD sed syntax, try to escape '' in Bash!
    else
        "$sedCmd" $sedOSCmdExt $sedCmdExt $additionalSedArgs "$@"
    fi
}

function preprocessFile()
{
    callSed \
        -e "$rewriteParams" \
        -e "s/^([ \x9]*)(\/\/){0,1}(.*)\/\/@(.*)/\1\3\/\/@\4/" \
        -e "/($defines)/!s/^([ \x9]*)(.*)\/\/(@.*)/\1\/\/\2\/\/\3/" \
        "$1"
}

function preprocessDirectory()
{
    "$findCmd" "$1" -type f | grep -v .svn | grep -v .git | while read file; do
        [[ "${file##*.}" == "qml" || "${file##*.}" == "js" ]] && preprocessFile "$file"
    done
}

function usage()
{
    cat << EOF
usage: $0 [options] <filename JS or QML or directoryname>

OPTIONS:
   -h                Show this message.
   -q <major.minor>  The version of QtQuick to rewrite to. Example: -q 2.1
   -d <defines>      The defines to set, separated by |. Example: -d "@QtQuick1|@Meego|@Debug"
   -i                Modify file in-place instead of dumping to stdout.
EOF
}

processInline=
additionalSedArgs=
defines=@NODEFINESET

while getopts ":hiq:d:" OPTION; do
    case $OPTION in
        q)
            rewriteQtQuickVersion="$OPTARG"
            ;;
        d)
            defines="$OPTARG"
            ;;
        i)
            processInline=true
            additionalSedArgs="-i"
            ;;
        h)
            usage
            exit 1
            ;;
        ?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            exit 1
            ;;
    esac
done

input="${@: -1}"

# Shortcuts for Kid3: 4, 5 and U for Qt4, Qt5 and Ubuntu.
if [ "$input" == "4" ]; then
        rewriteQtQuickVersion="1.1"
        defines="@QtQuick1|@!Ubuntu"
        processInline=true
        additionalSedArgs="-i"
        input="."
elif [ "$input" == "5" ]; then
        rewriteQtQuickVersion="2.2"
        defines="@QtQuick2|@!Ubuntu"
        processInline=true
        additionalSedArgs="-i"
        input="."
elif [ "$input" == "U" ]; then
        rewriteQtQuickVersion="2.2"
        defines="@QtQuick2|@Ubuntu"
        processInline=true
        additionalSedArgs="-i"
        input="."
fi
# End of modifications for Kid3, original script can be found at
# https://katastrophos.net/andre/blog/2013/09/20/qml-preprocessor-the-qnd-and-kiss-way/

rewriteParams=""
[ "$rewriteQtQuickVersion" != "" ] && rewriteParams="/\/\/!noRewrite/!s/(import QtQuick)[ \x9]*[0-9].[0-9]/\1 $rewriteQtQuickVersion/"

if [[ -f "$input" || "$input" == "-" ]]; then
        preprocessFile "$input"
elif [ -d "$input" ]; then
        if [ "$processInline" != "true" ]; then
            echo "Please specify -i when trying to preprocess a whole directory recursively."
            usage
            exit 1
        fi

        preprocessDirectory "$input"
else
        echo "Please specify a valid file or directory."
        usage
        exit 1
fi

exit 0
