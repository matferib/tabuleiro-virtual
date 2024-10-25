# proto_library, cc_proto_library, and java_proto_library rules implicitly
# depend on @com_google_protobuf for protoc and proto runtimes.
# This statement defines the @com_google_protobuf repo.
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
http_archive(
    name = "com_google_protobuf",
    #sha256 = "cef7f1b5a7c5fba672bec2a319246e8feba471f04dcebfe362d55930ee7c1c30",
    strip_prefix = "protobuf-3.6.1.2",
    urls = ["https://github.com/google/protobuf/archive/v3.6.1.2.zip"],
)

# QT
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_justbuchanan_rules_qt",
    remote = "https://github.com/justbuchanan/bazel_rules_qt.git",
    branch = "master",
)

load("//:qt_configure.bzl", "qt_configure")

qt_configure()

load("@local_config_qt//:local_qt.bzl", "local_qt_path")

new_local_repository(
    name = "qt",
    build_file = "@com_justbuchanan_rules_qt//:qt.BUILD",
    path = local_qt_path(),
)

load("@com_justbuchanan_rules_qt//tools:qt_toolchain.bzl", "register_qt_toolchains")
register_qt_toolchains()
