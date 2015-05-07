swig -c++ -javascript -outcurrentdir -node $1/DTC.i
swig -c++ -javascript -outcurrentdir -node $1/DTCLibTest.i
cp $1/* .
export INCLUDE_DIR=$1/..
node-gyp --debug configure build
mv ./build/Debug/DTC.node .
mv ./build/Debug/DTCLibTest.node .
#cp DTC.node ./web/modules/DTC/server/
#cp DTCLibTest.node ./web/modules/DTC/server/
rm -rf $1/build

