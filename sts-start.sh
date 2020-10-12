#!/bin/bash
touch ~/Short-Term\ Storage/README.txt
now=$(date)
echo "Run started at $now" > ~/Short-Term\ Storage/README.txt
./sts ~/Short-Term\ Storage 10 out.log err.log