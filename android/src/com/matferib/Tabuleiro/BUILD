load("@rules_android//rules:rules.bzl", "android_binary", "android_library")
load("@protobuf//bazel:java_lite_proto_library.bzl", "java_lite_proto_library")

android_binary(
    name = "tabvirt",
    manifest = "AndroidManifest.xml",
    resource_files = glob(["res/**"]),
    deps = [
        ":tabvirtlib",
        "//jni:jni",
    ],
    #debug_key = "tabuleiro-virtual.keystore",
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
        ":comum_java_proto",
        ":entidade_java_proto",
        ":tabuleiro_java_proto",
        "@protobuf//:protobuf_lite",
        "@protobuf//:protobuf_javalite",
    ],
)

java_lite_proto_library(
    name = "acoes_java_proto",
    deps = ["//jni/ent:acoes_proto"],
)

java_lite_proto_library(
    name = "comum_java_proto",
    deps = ["//jni/ent:comum_proto"],
)

java_lite_proto_library(
    name = "controle_virtual_java_proto",
    deps = ["//jni/ent:controle_virtual_proto"],
)

java_lite_proto_library(
    name = "entidade_java_proto",
    deps = ["//jni/ent:entidade_proto"],
    visibility = ["//visibility:public"],
)

java_lite_proto_library(
    name = "geracao_java_proto",
    deps = ["//jni/ent:geraco_proto"],
)

java_lite_proto_library(
    name = "tabelas_java_proto",
    deps = ["//jni/ent:tabelas_proto"],
)

java_lite_proto_library(
    name = "tabuleiro_java_proto",
    deps = ["//jni/ent:tabuleiro_proto"],
    visibility = ["//visibility:public"],
)

java_lite_proto_library(
    name = "tabuleiro_interface_java_proto",
    deps = ["//jni/ent:tabuleiro_interface_proto"],
)


