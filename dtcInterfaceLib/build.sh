swig -c++ -javascript -node DTC.i
swig -c++ -javascript -node DTCLibTest.i
node-gyp --debug configure build
mv build/Debug/DTC.node .
mv build/Debug/DTCLibTest.node .
cp DTC.node ./web/modules/DTC/server/
cp DTCLibTest.Node ./web/modules/DTC/server/
rm -rf build

