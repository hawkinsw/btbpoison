#!/bin/bash

total_misses=0
total_clears=0

iterations=200
for i in `seq 1 $iterations`; do
	output=`LD_LIBRARY_PATH=../libmsr/:$LD_LIBRARY_PATH ./poison`;
	current_misses=`echo $output | awk '{ difference=($5 - $15); if (difference<0) { difference=difference*-1; } printf("%f", difference);}'`
	total_misses=`echo "$total_misses+$current_misses" | bc`
	current_clears=`echo $output | awk '{ difference=($10 - $20); if (difference<0) { difference=difference*-1; } printf("%f", difference);}'`
	total_clears=`echo "$total_clears+$current_clears" | bc`
done

echo -n "Misses:" 
echo "scale=5; $total_misses/$iterations.0" | bc
echo -n "Clears:" 
echo "scale=5; $total_clears/$iterations.0" | bc
