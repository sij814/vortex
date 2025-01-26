#!/bin/bash

cd build
source ./ci/toolchain_env.sh

dirname=$(date +"%Y-%m-%d_%H-%M-%S")
mkdir "$dirname"

arbs=("0" "1" "2")
drivers=("simx" "rtlsim")

size=256
size_sq=$((size*size))

for arb in "${arbs[@]}"
do
    # simx or rtlsim
    for driver in "${drivers[@]}"
    do
        CONFIGS="-DPLATFORM_MEMORY_BANKS=4 -DL2_ARB_TYPE=$arb" ./ci/blackbox.sh --driver=$driver --app=blackscholes --cores=4 --clusters=2 --l2cache --rebuild=1 --perf=2 >> ${dirname}/output_${arb}.txt
        CONFIGS="-DPLATFORM_MEMORY_BANKS=4 -DL2_ARB_TYPE=$arb" ./ci/blackbox.sh --driver=$driver --app=conv3 --args='-n'"$size"'' --cores=4 --clusters=2 --l2cache --perf=2 >> ${dirname}/output_${arb}.txt
        CONFIGS="-DPLATFORM_MEMORY_BANKS=4 -DL2_ARB_TYPE=$arb" ./ci/blackbox.sh --driver=$driver --app=sgemm --args='-n'"$size"'' --cores=4 --clusters=2 --l2cache --perf=2 >> ${dirname}/output_${arb}.txt
        CONFIGS="-DPLATFORM_MEMORY_BANKS=4 -DL2_ARB_TYPE=$arb" ./ci/blackbox.sh --driver=$driver --app=transpose --args='-n'"$size"'' --cores=4 --clusters=2 --l2cache --perf=2 >> ${dirname}/output_${arb}.txt
        CONFIGS="-DPLATFORM_MEMORY_BANKS=4 -DL2_ARB_TYPE=$arb" ./ci/blackbox.sh --driver=$driver --app=vecadd --args='-n'"$size_sq"'' --cores=4 --clusters=2 --l2cache --perf=2 >> ${dirname}/output_${arb}.txt
    done
done