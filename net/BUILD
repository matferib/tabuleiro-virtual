package(default_visibility = ["//visibility:public"])

cc_library(
  name = "net",
  srcs = [
    "cliente.cpp",
    "servidor.cpp",
    "socket.cpp",
    "util.cpp",
  ],
  hdrs = [
    "cliente.h",
    "servidor.h",
    "socket.h",
    "util.h",
  ],
  copts = [],
  deps = [
      "@abseil-cpp//absl/strings",
      "@abseil-cpp//absl/strings:str_format",
      "//goog",
      "//log",
      "//ntf",
      "//ntf:ntf_cc_proto",
  ]
)
