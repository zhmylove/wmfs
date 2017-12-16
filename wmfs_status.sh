#!/bin/sh
# Made by: KorG

SLEEP=30

while true ;do

   STATUS="`date +%H:%M`"

   wmfs -s "$STATUS"
   sleep "$SLEEP"
done
