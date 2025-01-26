#!/bin/bash

cd build
source ./ci/toolchain_env.sh

dirname=$(date +"%Y-%m-%d_%H-%M-%S")
mkdir "$dirname"

mem_port=("1" "2" "4" "8")
drivers=("simx" "rtlsim")

size=32
size_sq=$((size*size))

# configuration
for port in "${mem_port[@]}"
do
    # simx or rtlsim
    for driver in "${drivers[@]}"
    do
        CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=bfs --args='../../../../tests/opencl/bfs/graph4k.txt' --cores=4 --clusters=2 --l2cache --rebuild=1 --perf=2 >> ${dirname}/output_port${port}.txt
        CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=conv3 --args='-n'"$size"'' --cores=4 --clusters=2 --l2cache --perf=2 >> ${dirname}/output_port${port}.txt
        CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=sgemm --args='-n'"$size"'' --cores=4 --clusters=2 --l2cache --perf=2 >> ${dirname}/output_port${port}.txt
        CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=transpose --args='-n'"$size"'' --cores=4 --clusters=2 --l2cache --perf=2 >> ${dirname}/output_port${port}.txt
        CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=vecadd --args='-n'"$size_sq"'' --cores=4 --clusters=2 --l2cache --perf=2 >> ${dirname}/output_port${port}.txt
    done
done