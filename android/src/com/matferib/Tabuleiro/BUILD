load("@rules_android//rules:rules.bzl", "android_binary", "android_library")

android_binary(
    name = "tabvirt",
    manifest = "AndroidManifest.xml",
    resource_files = glob(["res/**"]),
    deps = [
        ":tabvirtlib",
    ],
)

android_library(
    name = "tabvirtlib",
    srcs = [
        "TabuleiroActivity.java",
        "SelecaoActivity.java",
    ],
    manifest = "AndroidManifest.xml",
    resource_files = glob(["res/**"]),
    deps = [
        "//jni:jni",
        "//jni/ent:entidade_java_proto",
        "//jni/ent:tabuleiro_java_proto",
        "@protobuf//:protobuf_java",
    ],
)
