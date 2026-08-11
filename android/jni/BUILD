load("@rules_android//rules:rules.bzl", "android_binary", "android_library")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "jni",
    srcs = ["jni-impl.cpp"],
    cxxopts = [
      "-Ijni",
    ],
    defines = [
      "ANDROID=1",
      "USAR_OPENGL_ES=1",
      "DEBUG=0",
      "BENCHMARK=0",
      #"-Wno-deprecated-declarations",
    ],
    deps = [
      "@boost//:asio",
      "@boost//:filesystem",
      "//jni/arq",
      "//jni/ent:ent_headers",
      "//jni/ifg",
      "//jni/m3d",
      "//jni/net",
    ],
    #linkopts = select({
      #"//conditions:default": [],
    #})
)

