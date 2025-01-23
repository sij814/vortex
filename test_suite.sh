#!/bin/bash

cd build
source ./ci/toolchain_env.sh

list=("conv3" "dotproduct" "sgemm" "transpose" "vecadd")
mem_port=("2" "4" "8")
clusters=("2" "4" "8")
drivers=("simx" "rtlsim")

size=16
size_sq=$((size*size))

# configuration
for cluster in "${clusters[@]}"
do
    cores=$((32/cluster))
    # number of mem_ports
    for port in "${mem_port[@]}"
    do
        rm output_${cluster}_${port}.txt

        # simx or rtlsim
        for driver in "${drivers[@]}"
        do
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=conv3 --args='-n'"$size"'' --cores=$cores --clusters=$cluster --l2cache --rebuild=1 --perf=2 >> output_${cluster}_${port}.txt
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=dotproduct --args='-n'"$size_sq"'' --cores=$cores --clusters=$cluster --l2cache --perf=2 >> output_${cluster}_${port}.txt
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=sgemm --args='-n'"$size"'' --cores=$cores --clusters=$cluster --l2cache --perf=2 >> output_${cluster}_${port}.txt
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=transpose --args='-n'"$size"'' --cores=$cores --clusters=$cluster --l2cache --perf=2 >> output_${cluster}_${port}.txt
            CONFIGS="-DPLATFORM_MEMORY_BANKS=$port" ./ci/blackbox.sh --driver=$driver --app=vecadd --args='-n'"$size_sq"'' --cores=$cores --clusters=$cluster --l2cache --perf=2 >> output_${cluster}_${port}.txt
        done
    done
done