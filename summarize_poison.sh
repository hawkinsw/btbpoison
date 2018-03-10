#!/bin/bash

total=0

iterations=200
for i in `seq 1 $iterations`; do
	output=`LD_LIBRARY_PATH=../libmsr/:$LD_LIBRARY_PATH ./poison`;
	current=`echo $output | awk '{ difference=($5 - $10); if (difference<0) { difference=difference*-1; } printf("%f", difference);}'`
	total=`echo "$total+$current" | bc`
done

echo "scale=5; $total/$iterations.0" | bc
