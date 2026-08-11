load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "arq_headers",
  hdrs = [
    "arquivo.h",
  ],
  copts = ["-Ijni"],
)

cc_library(
  name = "arq",
  srcs = [
    "arquivo.cpp",
    "arquivo_android.cpp",
  ],
  hdrs = [
    "arquivo.h",
  ],
  copts = ["-Ijni"],
  deps = [
    "@protobuf//:protobuf",
    "@boost//:filesystem",
    "//jni/log:log",
  ]
)
