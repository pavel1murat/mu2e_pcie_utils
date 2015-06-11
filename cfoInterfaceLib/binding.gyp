{
 "targets": [
  {
    "target_name": "CFO",
    "sources": ["CFO_Types.cc", "CFO.cc", "CFO_wrap.cxx", 
"mu2edev.cc",
"mu2esim.cc" ],
    "include_dirs": ["/${INCLUDE_DIR}", "/${TRACE_INC}"],
    "cflags": [
      "-std=c++11",
      "-fexceptions"
    ],
    "cflags_cc": [
      "-std=c++11",
      "-fexceptions"
    ]
  },
  {
    "target_name": "CFOLibTest",
    "sources": ["CFOLibTest.cc",
 "CFOLibTest_wrap.cxx",
 "CFO_Types.cc",
 "CFO.cc",
 "mu2edev.cc",
 "mu2esim.cc"],
    "include_dirs": ["/${INCLUDE_DIR}", "/${TRACE_INC}"],
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
