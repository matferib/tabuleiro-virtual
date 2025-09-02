package(default_visibility = ["//visibility:public"])

cc_library(
  name = "log",
  hdrs = [
    "log.h",
  ],
  defines= ['USAR_GLOG'],
  deps = [
    "@abseil-cpp//absl/flags:flag",
    "@abseil-cpp//absl/flags:parse",
    "@abseil-cpp//absl/log",
    "@abseil-cpp//absl/log:initialize",
    "@abseil-cpp//absl/strings",
  ]
)
