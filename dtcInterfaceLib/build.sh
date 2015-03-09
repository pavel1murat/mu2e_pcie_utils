swig -c++ -javascript -node DTC.i
node-gyp --debug configure build
mv build/Debug/DTC.node .
cp DTC.node ./web/modules/DTC/server/
rm -rf build

