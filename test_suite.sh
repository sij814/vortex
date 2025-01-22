cd build

rm output.txt

make -s

./ci/blackbox.sh --driver=simx --app=blackscholes --args='-n16384' --cores=8 --clusters=8 --l2cache --l3cache --rebuild=1 --perf=2 >> output.txt
./ci/blackbox.sh --driver=simx --app=conv3 --args='-n16384' --cores=8 --clusters=8 --l2cache --l3cache --perf=2 >> output.txt
./ci/blackbox.sh --driver=simx --app=saxpy --args='-n16384' --cores=8 --clusters=8 --l2cache --l3cache --perf=2 >> output.txt
./ci/blackbox.sh --driver=simx --app=sgemm --args='-n16384' --cores=8 --clusters=8 --l2cache --l3cache --perf=2 >> output.txt
./ci/blackbox.sh --driver=simx --app=sgemm3 --args='-n16384' --cores=8 --clusters=8 --l2cache --l3cache --perf=2 >> output.txt
./ci/blackbox.sh --driver=simx --app=stencil3d --args='-n16384' --cores=8 --clusters=8 --l2cache --l3cache --perf=2 >> output.txt
./ci/blackbox.sh --driver=simx --app=vecaddx --args='-n16384' --cores=8 --clusters=8 --l2cache --l3cache --perf=2 >> output.txt
