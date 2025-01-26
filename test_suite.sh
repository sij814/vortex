#!/bin/bash

cd build
source ./ci/toolchain_env.sh

dirname=$(date +"%Y-%m-%d_%H-%M-%S")
mkdir "$dirname"

mem_port=("1" "2" "4" "8")
clusters=("4" "8")
drivers=("simx" "rtlsim")

size=256
size_sq=$((size*size))

# configuration
for port in "${mem_port[@]}"
do
    make clean
    make -s
    # number of mem_ports
    for cluster in "${clusters[@]}"
    do
        cores=$((32/cluster))
        # simx or rtlsim
        for driver in "${drivers[@]}"
        do
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=blackscholes --cores=$cores --clusters=$cluster --l2cache --rebuild=1 --perf=2 >> ${dirname}/output_${cluster}_${port}.txt
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=conv3 --args='-n'"$size"'' --cores=$cores --clusters=$cluster --l2cache --perf=2 >> ${dirname}/output_${cluster}_${port}.txt
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=sgemm --args='-n'"$size"'' --cores=$cores --clusters=$cluster --l2cache --perf=2 >> ${dirname}/output_${cluster}_${port}.txt
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=transpose --args='-n'"$size"'' --cores=$cores --clusters=$cluster --l2cache --perf=2 >> ${dirname}/output_${cluster}_${port}.txt
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=vecadd --args='-n'"$size_sq"'' --cores=$cores --clusters=$cluster --l2cache --perf=2 >> ${dirname}/output_${cluster}_${port}.txt
        done
    done
done