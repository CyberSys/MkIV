#!/bin/bash
# 100 runs per batch
for i in {0..99} 
do
	# twenty batches of time steps
	for timestep in 0.2 0.4 0.8 1.0 1.2 1.4 1.6 1.8 2.0 2.2 2.4 2.6 2.8 3.0 3.2 3.4 3.6 3.8 4.0 5.0
	do 
		echo "Batch #$timestep Run #$i"
		sleep 5
		./markiv --settings ./scripts/crowdsettings.ini --runid $i --nogfxprompt --timecomp $timestep --batchout crowd/x$timestep-out-b.txt
		sleep 5
	done
done
