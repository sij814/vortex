cd build
source ./ci/toolchain_env.sh

rm output.txt

make -s

./ci/blackbox.sh --driver=simx --app=blackscholes --args='-n64' --cores=8 --clusters=4 --l2cache --l3cache --rebuild=1 --perf=2 >> output.txt #2D
./ci/blackbox.sh --driver=simx --app=conv3 --args='-n64' --cores=8 --clusters=4 --l2cache --l3cache  --perf=2 >> output.txt #2D
./ci/blackbox.sh --driver=simx --app=saxpy --args='-n4096' --cores=8 --clusters=4 --l2cache --l3cache  --perf=2 >> output.txt #1D
./ci/blackbox.sh --driver=simx --app=sgemmx --args='-n64' --cores=8 --clusters=4 --l2cache --l3cache --perf=2 >> output.txt #2D
./ci/blackbox.sh --driver=simx --app=sgemm2x --args='-n64' --cores=8 --clusters=4 --l2cache --l3cache --perf=2 >> output.txt #2D
./ci/blackbox.sh --driver=simx --app=vecaddx --args='-n4096' --cores=8 --clusters=4 --l2cache --l3cache --perf=2 >> output.txt #1D
