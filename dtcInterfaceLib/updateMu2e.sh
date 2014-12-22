swig -c++ -javascript -node DTC.i
node-gyp --debug configure build
scp build/Debug/DTC.node mu2edaq01.dhcp:~/dev/server
scp web/client/* mu2edaq01.dhcp:~/dev/client
scp web/server/* mu2edaq01.dhcp:~/dev/server
scp web/serverbase.js mu2edaq01.dhcp:~/dev
ssh mu2edaq01.dhcp "touch ~/dev/.devbroken"
