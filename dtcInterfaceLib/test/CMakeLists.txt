cet_make_exec(packetGenerator SOURCE packetGenerator.cc)
cet_make_exec(packetReader SOURCE packetReader.cc)

cet_make_exec(DTCLibTest SOURCE DTCLibTest.cc LIBRARIES DTCInterface)

cet_make_exec(tester SOURCE tester.cc LIBRARIES DTCInterface)

cet_make_exec(sizeof_buffdesc SOURCE sizeof_buffdesc.cc LIBRARIES DTCInterface)

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()

cet_script(testdata.bin)
