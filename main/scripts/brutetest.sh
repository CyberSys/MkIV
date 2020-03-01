#!/bin/bash
# 100 runs per batch
for i in {0..99} 
do
	# rules input sets (generate these)
	for ruleset in {0..71}
	do 
		echo "Batch #$ruleset Run #$i"
		./markiv --settings ./scripts/bruteforce.ini --runid $i --nogfxprompt --fuzzyrules ai/brute-rules$ruleset.txt --batchout brute/rule$ruleset-out.txt
		sleep 5
	done
done
