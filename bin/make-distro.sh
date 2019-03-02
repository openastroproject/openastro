#!/bin/bash

set -e

if [ "$1" == "" ]
then
  echo "usage: make-distro.sh <source-dir> <version> <dest-dir>"
  exit 1
fi

source=$1
version=$2
distroDir=$3
distroName=oacapture-$version
distroPath=$distroDir/$distroName

if [ ! -d $source ]
then
  echo "$source does not exist"
  exit 2
fi

if [ ! -d $distroDir ]
then
  echo "$distroDir does not exist"
  exit 2
fi

rm -fr $distroPath
rsync -a $source/ $distroPath
if [ $? -ne 0 ]
then
  echo "copy of core distribution to $distroPath failed"
  exit 4
fi

for f in `cat $source/.gitignore | sed -e '/^#/d' -e '/^[ 	]*$/d' -e '/^$/d' | sort -u`
do
  case $f in
    /*)
      rm -fr $distroPath$f
      ;;
    *)
      find $distroPath -name "$f" -print | egrep -v 'ffmpeg/(configure|.*Makefile)$' | xargs rm -fr
      ;;
  esac
done

find $distroPath -type l -name \*.a -print | xargs rm -f
find $distroPath -type l -name \*.so -print | xargs rm -f
cd $distroPath
rm -fr .git
./bootstrap
cd $distroDir
tar cf $distroName.tar ./$distroName
rm -f $distroName.tar.bz2
bzip2 $distroName.tar
