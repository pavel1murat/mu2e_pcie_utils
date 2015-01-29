{
 "targets": [
  {
    "target_name": "DTC",
    "sources": ["DTC_Types.cc", "DTC.cc", "DTC_wrap.cxx", "mu2edev.cc", "mu2esim.cc" ],
    "include_dirs": ["../linux_driver/mymodule2", "/${TRACE_INC}"],
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
