#!/bin/sh

JAVA_OPTS="-Xss10m -Dawt.useSystemAAFontSettings=on"

#Following symlinks
path=$0
while [ -h "$path" ]; do
    path=$(ls -ld $path | sed -r 's/^.*->.//')
done

java $JAVA_OPTS -jar $(dirname $path)/legio.jar "$@"

