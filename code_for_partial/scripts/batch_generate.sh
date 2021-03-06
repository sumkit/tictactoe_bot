#/usr/bin/env bash
# generate jobs in batch

threads=(8 16) # The number of threads 
rm -f *.job

for t in ${threads[@]}
  do
    ../scripts/generate_jobs.sh $t
  done
