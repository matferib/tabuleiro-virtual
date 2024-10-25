package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ntf_headers",
  hdrs = [
    "notificacao.h",
  ],
  copts = [],
  deps = [
    ":ntf_cc_proto",
  ]
)

cc_library(
  name = "ntf",
  srcs = [
    "notificacao.cpp",
  ],
  hdrs = [
    "notificacao.h",
  ],
  copts = [],
  deps = [
    ":ntf_cc_proto",
    "//log",
  ]
)

cc_proto_library(
  name = "ntf_cc_proto",
  deps = [":ntf_proto"]
)

proto_library(
  name = "ntf_proto",
  srcs = [
    "notificacao.proto",
  ],
  deps = [
    "//ent:entidade_proto",
    "//ent:tabelas_proto",
    "//ent:tabuleiro_proto",
  ]
)
