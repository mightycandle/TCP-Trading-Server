#!/bin/bash

fullfile= $1
filename= $(basename -- "$fullfile")
filename="${filename%.*}"

g++ $fullfile -o $filename;
./$filename;
