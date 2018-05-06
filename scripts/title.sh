#!/bin/sh
# Made by: KorG

TITLE=`xprop |sed -n '/^WM_NAME/s/^.* = "\(.*\)"$/\1/p'`

wmfs -s "Title: $TITLE "
