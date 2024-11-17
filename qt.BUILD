load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

QT_LIBRARIES = [
    ("core", "QtCore", "Qt6Core", []),
    ("network", "QtNetwork", "Qt6Network", []),
    ("widgets", "QtWidgets", "Qt6Widgets", [":qt_core", ":qt_gui"]),
    ("quick", "QtQuick", "Qt6Quick", [":qt_gui", ":qt_qml"]),
    ("qml", "QtQml", "Qt6Qml", [":qt_core", ":qt_network"]),
    ("qml_models", "QtQmlModels", "Qt6QmlModels", []),
    ("gui", "QtGui", "Qt6Gui", [":qt_core"]),
    ("opengl", "QtOpenGL", "Qt6OpenGL", []),
    ("openglwidgets", "QtOpenGLWidgets", "Qt6OpenGLWidgets", []),
    ("multimedia", "QtMultimedia", "Qt6Multimedia", []),
]

[
    cc_library(
        name = "qt_%s_linux" % name,
        # When being on Windows this glob will be empty
        hdrs = glob(["%s/**" % include_folder], allow_empty = True),
        includes = ["."],
        linkopts = ["-l%s" % library_name],
        # Available from Bazel 4.0.0
        # target_compatible_with = ["@platforms//os:linux"],
    )
    for name, include_folder, library_name, _ in QT_LIBRARIES
]

[
    cc_import(
        name = "qt_%s_windows_import" % name,
        # When being on Linux or macOS this glob will be empty
        hdrs = glob(["include/%s/**" % include_folder], allow_empty = True),
        interface_library = "lib/%s.lib" % library_name,
        shared_library = "bin/%s.dll" % library_name,
        # Not available in cc_import (See: https://github.com/bazelbuild/bazel/issues/12745)
        # target_compatible_with = ["@platforms//os:windows"],
    )
    for name, include_folder, library_name, _ in QT_LIBRARIES
]

[
    cc_library(
        name = "qt_%s_windows" % name,
        # When being on Linux or macOS this glob will be empty
        hdrs = glob(["include/%s/**" % include_folder], allow_empty = True),
        includes = ["include"],
        # Available from Bazel 4.0.0
        # target_compatible_with = ["@platforms//os:windows"],
        deps = [":qt_%s_windows_import" % name],
    )
    for name, include_folder, _, _ in QT_LIBRARIES
]

[
    cc_library(
        name = "qt_%s_osx" % name,
        # When being on Windows or Linux this glob will be empty
        hdrs = glob(["include/%s/**" % include_folder], allow_empty = True),
        includes = ["include", "."],
        # macOS qt libs do not contain a 6 - e.g. instead of Qt6Core the lib is called QtCore
        # Qt5 does not have OpenGLWidgets
        linkopts = [
            "-framework %s" % library_name.replace("6", "") if library_name != "Qt6OpenGLWidgets" else "",
            ],
        # Available from Bazel 4.0.0
        # target_compatible_with = ["@platforms//os:osx"],
    )
    for name, include_folder, library_name, _ in QT_LIBRARIES
]

[
    cc_library(
        name = "qt_%s" % name,
        visibility = ["//visibility:public"],
        deps = dependencies + select({
            "@platforms//os:linux": [":qt_%s_linux" % name],
            "@platforms//os:windows": [":qt_%s_windows" % name],
            "@platforms//os:osx": [":qt_%s_osx" % name],
        }),
    )
    for name, _, _, dependencies in QT_LIBRARIES
]

# TODO: Make available also for Windows
cc_library(
    name = "qt_3d",
    # When being on Windows this glob will be empty
    hdrs = glob([
        "Qt3DAnimation/**",
        "Qt3DCore/**",
        "Qt3DExtras/**",
        "Qt3DInput/**",
        "Qt3DLogic/**",
        "Qt3DQuick/**",
        "Qt3DQuickAnimation/**",
        "Qt3DQuickExtras/**",
        "Qt3DQuickInput/**",
        "Qt3DQuickRender/**",
        "Qt3DQuickScene2D/**",
        "Qt3DRender/**",
    ], allow_empty = True),
    includes = ["."],
    linkopts = [
        "-lQt63DAnimation",
        "-lQt63DCore",
        "-lQt63DExtras",
        "-lQt63DInput",
        "-lQt63DLogic",
        "-lQt63DQuick",
        "-lQt63DQuickAnimation",
        "-lQt63DQuickExtras",
        "-lQt63DQuickInput",
        "-lQt63DQuickRender",
        "-lQt63DQuickScene2D",
        "-lQt63DRender",
    ],
    # Available from Bazel 4.0.0
    # target_compatible_with = ["@platforms//os:linux"],
)

# TODO: remove in favor of toolchain-defined binary
filegroup(
    name = "uic",
    srcs = ["bin/uic.exe"],
    visibility = ["//visibility:public"],
)

# TODO: remove in favor of toolchain-defined binary
filegroup(
    name = "moc",
    srcs = ["bin/moc.exe"],
    visibility = ["//visibility:public"],
)
