package(default_visibility = ["//visibility:public"])

cc_library(
  name = "arq_headers",
  hdrs = [
    "arquivo.h",
  ],
  copts = [],
)

cc_library(
  name = "arq",
  srcs = [
    "arquivo.cpp",
    "arquivo_pc.cpp",
  ],
  hdrs = [
    "arquivo.h",
  ],
  copts = [],
  deps = [
    "@protobuf//:protobuf",
    "@boost//:filesystem",
    "//log:log",
  ]
)
