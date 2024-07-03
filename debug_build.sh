cd third_party/ramulator2
'rm libramulator.so'
'rm -rf build'
'mkdir build'
cd build
cmake ..
make -j4
cd ../../../build
make clean
../configure --xlen=32 --tooldir=$HOME/tools
make -s
./ci/blackbox.sh --cores=2 --app=vecadd