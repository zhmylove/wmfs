#!/bin/sh
# Made by: KorG

SLEEP=15

while true ;do

   STATUS="`date +%H:%M`"

   wmfs -s "$STATUS"
   sleep "$SLEEP"
done
