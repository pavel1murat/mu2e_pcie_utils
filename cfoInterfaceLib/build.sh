#! /bin/sh
. `ups unsetup python`
cp $1/* .
swig -c++ -javascript -outcurrentdir -node $1/CFO.i
swig -c++ -javascript -outcurrentdir -node $1/CFOLibTest.i
export INCLUDE_DIR=$1/..
node-gyp --debug configure build
mv ./build/Debug/CFO.node .
mv ./build/Debug/CFOLibTest.node .
rm -rf $1/build

