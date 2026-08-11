load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "tex_headers",
  hdrs = [
    "texturas.h",
    "lodepng.h",
  ],
  copts = ["-Ijni"],
  deps = [
  ]
)

cc_library(
  name = "tex",
  hdrs = [
    "texturas.h",
    "lodepng.h",
  ],
  srcs = [
    "lodepng.cpp",
    "texturas.cpp",
  ],
  copts = ["-Ijni"],
  deps = [
    "//jni/arq",
    "//jni/ent:ent_headers",
    "//jni/ent:entidade_cc_proto",
    "//jni/gltab:gltab",
    "//jni/log:log",
    "//jni/ntf:ntf_cc_proto",
  ]
)
