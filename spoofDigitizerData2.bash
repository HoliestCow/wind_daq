#!/bin/sh

#  spoofDigitizerData.bash
#  
#
#  Created by Callie Goetz on 8/23/17.
#


#set filename want here
inputFile="channel1.txt"

#get rid of old one in case it already exists
rm $inputFile

#write header info to input file
echo "HEADER0:1537" >> $inputFile
echo "HEADER1:1280" >> $inputFile
echo "HEADER2:769" >> $inputFile
echo "HEADER3:1282" >> $inputFile
echo "HEADER4:771" >> $inputFile
echo "HEADER5:33540" >> $inputFile

#set count = 0
count=0

#writes one line a second until stopped
while true
do

#appends line with count number and space between, e.g. 5 5 5 5
#increment count so easy to keep track of lines being added
echo $count" "$count" "$count" "$count >> $inputFile

#iterate count
count=`expr $count + 1`

#wait one second
sleep 0.1
done
