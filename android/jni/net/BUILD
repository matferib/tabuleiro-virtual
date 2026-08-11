load("@rules_cc//cc:defs.bzl", "cc_library")

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
  copts = ["-Ijni"],
  deps = [
      "@abseil-cpp//absl/strings",
      "@abseil-cpp//absl/strings:str_format",
      "@boost//:algorithm",
      "@boost//:asio",
      "@boost//:timer",
      "//jni/log:log",
      "//jni/ntf:ntf",
      "//jni/ntf:ntf_cc_proto",
  ]
)
