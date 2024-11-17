package(default_visibility = ["//visibility:public"])

load("//:qt.bzl", "qt_cc_library", "qt_ui_library")

cc_library(
  name = "som",
  srcs = [
    "som_qt.cpp",
  ],
  hdrs = [
    "som.h",
  ],
  copts = [],
  deps = [
      "@abseil-cpp//absl/strings:str_format",
      "@qt//:qt_core",
      "@qt//:qt_gui",
      "@qt//:qt_multimedia",
      "//arq:arq_headers",
      "//ent:tabuleiro_cc_proto",
      "//log:log",
  ]
)
