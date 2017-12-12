#/usr/bin/env bash

# generate the job for latedays
threads=$1

if [ ${#} -ne 1 ]; then
  echo "Usage: $0 <threads>"
else
  curdir=`pwd`
  curdir=${curdir%/templates}
  sed "s:PROGDIR:${curdir}:g" ../scripts/example.job.template > tmp1.job
  sed "s:THREADS:${threads}:g" tmp1.job > ${USER}_${threads}.job
  rm -f tmp1.job
fi
