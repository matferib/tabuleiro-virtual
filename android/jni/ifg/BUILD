load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_library")
load("@protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ifg_headers",
  hdrs = [
    "interface.h",
    "tecladomouse.h",
  ],
  copts = ["-Ijni"],
  deps = [
    ":ifg_cc_proto",
    "//jni/arq:arq_headers",
  ]
)


cc_library(
  name = "ifg",
  srcs = [
      "interface.cpp",
      "interface_android.cpp",
      "tecladomouse.cpp",
  ],
  hdrs = [
      "interface.h",
      "interface_android.h",
      "tecladomouse.h",
  ],
  copts = ["-Ijni"],
  deps = [
      "@abseil-cpp//absl/strings",
      "@boost//:timer",
      ":ifg_cc_proto",
      "//jni/arq:arq",
      "//jni/ent:ent_headers",
      "//jni/ntf:ntf",
      "//jni/tex:tex",
  ]
)

cc_proto_library(
  name = "ifg_cc_proto",
  deps = [
    "ifg_proto"
  ],
)

proto_library(
  name = "ifg_proto",
  srcs = ["modelos.proto"],
  strip_import_prefix = "/jni"
)
