#! /bin/sh
. `ups unsetup python`
cp $1/* .
swig -c++ -javascript -outcurrentdir -node $1/DTC.i
swig -c++ -javascript -outcurrentdir -node $1/DTCLibTest.i
export INCLUDE_DIR=$1/..
node-gyp --debug configure build
mv ./build/Debug/DTC.node .
mv ./build/Debug/DTCLibTest.node .
#cp DTC.node ./web/modules/DTC/server/
#cp DTCLibTest.node ./web/modules/DTC/server/
rm -rf $1/build

