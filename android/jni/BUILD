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
      # defines so propagam de deps para pai, então não adianta definir aqui (teria que definir nas regras). Preferi colocar no .bazelrc para valer globalmente.
    ],
    linkopts = [
      "-landroid",
      "-lEGL",
      "-lGLESv3",
      "-laaudio",
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

