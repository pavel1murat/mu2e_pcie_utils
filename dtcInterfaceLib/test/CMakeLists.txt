cet_make_exec(packetGenerator SOURCE packetGenerator.cc)
cet_make_exec(packetReader SOURCE packetReader.cc)

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()

cet_script(testdata.bin)
