package(default_visibility = ["//visibility:public"])

cc_library(
  name = "tex_headers",
  hdrs = [
    "texturas.h",
    "lodepng.h",
  ],
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
  copts = [],
  deps = [
    "//arq",
    "//ent:ent_headers",
    "//ent:entidade_cc_proto",
    "//gltab:gltab",
    "//log:log",
    "//ntf:ntf_cc_proto",
  ]
)
