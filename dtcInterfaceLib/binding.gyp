{
  "targets": [
    {
      "target_name": "DTC",
      "sources": [
        "DTC_Types.cc",
        "DTC_Packets.cc",
        "DTC_Registers.cc",
        "DTC.cc",
        "DTC_wrap.cxx",
        "mu2edev.cc",
        "mu2esim.cc"
      ],
      "include_dirs": [ "/${INCLUDE_DIR}", "/${TRACE_DIR}/include" ],
      "cflags": [
        "-std=c++11",
        "-fexceptions"
      ],
      "cflags_cc": [
        "-std=c++11",
        "-fexceptions"
      ]
    }
  ]
}
