load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "som",
  srcs = [
    "som.cpp",
  ],
  hdrs = ["som.h", "stb_vorbis.cpp"],
  deps = [
    "//jni/ent:ent_headers",
    "//jni/log:log",
  ],
  copts = ["-Ijni"],
)
