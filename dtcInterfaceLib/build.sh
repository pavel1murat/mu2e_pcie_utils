swig -c++ -javascript -node DTC.i
node-gyp --debug configure build
mv build/Debug/DTC.node .
rm -rf build

