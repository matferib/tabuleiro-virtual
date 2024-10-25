load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library", "qt_ui_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_binary(
    name = "tabvirt",
    srcs = ["main.cpp"],
    copts = [
      "-fpic",
    ],
    deps = [
      "@abseil-cpp//absl/strings",
      "@qt//:qt_core",
      "@qt//:qt_widgets",
      "//ent:ent",
      "//ifg:ifg",
      "//ifg/qt:ifg_qt",
      "//log:log",
      "//m3d:m3d",
      "//net:net",
      "//ntf:ntf",
      "//som:som",
      "//tex:tex",
    ],
    linkopts = [
      '-lGLU',
      '-lGL',
      '-lboost_timer',
      '-lboost_chrono',
      '-lboost_filesystem',
      '-lboost_system',
      '-lboost_date_time',
      '-lpthread',
      #'-ltbb',
    ],
)
