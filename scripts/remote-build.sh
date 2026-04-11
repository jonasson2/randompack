#!/bin/bash
for remote in pluto elja spark1; do
  echo
  echo BUILDING ON HOST $remote:
  ssh $remote "cd randompack && git pull && ninja -C release install"
done
