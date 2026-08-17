load("@rules_android//rules:rules.bzl", "android_binary", "android_library")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

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
    linkopts = [
      "-landroid",
      "-lEGL",
      "-lGLESv3",
    ],
    deps = [
      "@boost//:asio",
      "@boost//:filesystem",
      "//jni/arq",
      "//jni/ent",
      "//jni/gltab",
      "//jni/ifg",
      "//jni/m3d",
      "//jni/net",
    ],
)

