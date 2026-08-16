load("@rules_android//rules:rules.bzl", "android_binary", "android_library")

platform(
    name = "arm64-v8a",
    constraint_values = [
        "@platforms//cpu:arm64",
        "@platforms//os:android",
    ],
)

platform(
    name = "x86",
    constraint_values = [
        "@platforms//cpu:x86_32",
        "@platforms//os:android",
    ],
)

#android_binary(
#    name = "app",
#    manifest = "AndroidManifest.xml",
#    deps = [
#        ":applib",
#    ],
#)
