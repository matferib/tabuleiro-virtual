load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "som",
  hdrs = [
    "som.h",
  ],
  deps = [
     "//ent:tabuleiro_cc_proto",
  ]
)

cc_library(
  name = "som_dummy",
  srcs = [
    "som_dummy.cpp",
  ],
  deps = [
      ":som",
  ]
)
