#!/bin/bash

#
# This runs time_mispredict_c 1001 times and calculates an average of the time
# for the jump to execute when the predictor is right (hot)
# and when the predictor is wrong (cold). We reinvoke so that the 
# btb does not learn about both targets and start overpredicting.
#

for i in `seq 0 1000`; do 
	./time_mispredict_c
done | awk -F: "
		BEGIN{
			cold_total = 0; cold_iterations = 0; hot_total = 0; hot_iterations = 0;
		} 
		/^cold/ {cold_total+=\$2; cold_iterations+=1;}
		/^hot/ {hot_total+=\$2; hot_iterations+=1;}
		END{ 
			cold_average = ((cold_total*1.0)/cold_iterations);
			hot_average = ((hot_total*1.0)/hot_iterations);
			printf \"cold_iterations: %d\n\", cold_iterations; 
			printf \"hot_iterations: %d\n\", hot_iterations;
			printf \"hot_total: %d\n\", hot_total;
			printf \"cold_total: %d\n\", cold_total;
			printf \"hot average : %f\n\", hot_average;
			printf \"cold average: %f\n\", cold_average;
			printf \"difference  : %f\n\", (((hot_average-cold_average)/cold_average)*100.0);
		}";
