load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "m3d",
  hdrs = [
    "m3d.h",
  ],
  srcs = [
    "m3d.cpp",
  ],
  copts = ["-Ijni"],
  deps = [
    "//jni/arq:arq",
    "//jni/ent:ent_headers",
    "//jni/ent:acoes_cc_proto",
    "//jni/ent:controle_virtual_cc_proto",
    "//jni/ent:entidade_cc_proto",
    "//jni/ent:tabelas_cc_proto",
    "//jni/ent:tabuleiro_cc_proto",
    "//jni/gltab:gltab",
    "//jni/log:log",
    "//jni/ntf:ntf_cc_proto",
  ]
)
