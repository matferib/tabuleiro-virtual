package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ifg_headers",
  hdrs = [
    "interface.h",
    "tecladomouse.h",
  ],
  copts = [],
  deps = [
    ":ifg_cc_proto",
    "//arq:arq_headers",
  ]
)


cc_library(
  name = "ifg",
  srcs = [
      "interface.cpp",
      "tecladomouse.cpp",
  ],
  hdrs = [
      "interface.h",
      "tecladomouse.h",
  ],
  copts = [],
  deps = [
      "@abseil-cpp//absl/strings",
      ":ifg_cc_proto",
      "//arq:arq",
      "//ent:ent_headers",
      "//ntf:ntf",
      "//tex:tex",
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
)
