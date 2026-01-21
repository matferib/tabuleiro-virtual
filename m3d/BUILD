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
  copts = [],
  deps = [
    "//arq:arq",
    "//ent:ent_headers",
    "//ent:acoes_cc_proto",
    "//ent:controle_virtual_cc_proto",
    "//ent:entidade_cc_proto",
    "//ent:tabelas_cc_proto",
    "//ent:tabuleiro_cc_proto",
    "//gltab:gltab",
    "//log:log",
    "//ntf:ntf_cc_proto",
  ]
)
