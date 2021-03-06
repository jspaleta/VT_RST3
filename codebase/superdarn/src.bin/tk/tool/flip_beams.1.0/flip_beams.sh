#!/bin/bash -x
cd /sd-data/2011/rawacf/bks
for (( i = 0 ; i <= 61; i++ ))
do
  date=$((`date --date="1 Sep 2011 +$i days" +%Y%m%d`))
  echo $date
  for fname in $date*
  do
		echo $fname
		l=`expr length $fname`
		echo $l
		end=$(( $l - 4 ))
		echo $(( $l - 4 ))
		fname2=${fname:0:end}
		echo $fname2
		cp $fname $fname2.backwards.bz2
		bunzip2 $fname -c > temp.rawacf
		rm $fname
		flip_beams temp.rawacf > $fname2
		bzip2 $fname2
		rm temp.rawacf
	done
done