source ../products/setups
setup gcc
setup swig
swig -c++ -javascript -node DTC.i
node-gyp --debug configure build
