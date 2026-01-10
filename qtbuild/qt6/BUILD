load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

QT_LIBRARIES = [
    ("core", "QtCore", "Qt6Core", []),
    ("network", "QtNetwork", "Qt6Network", []),
    ("widgets", "QtWidgets", "Qt6Widgets", [":qt6_core", ":qt6_gui"]),
    ("quick", "QtQuick", "Qt6Quick", [":qt6_gui", ":qt6_qml"]),
    ("qml", "QtQml", "Qt6Qml", [":qt6_core", ":qt6_network"]),
    ("qml_models", "QtQmlModels", "Qt6QmlModels", []),
    ("gui", "QtGui", "Qt6Gui", [":qt6_core"]),
    ("opengl", "QtOpenGL", "Qt6OpenGL", []),
    ("openglwidgets", "QtOpenGLWidgets", "Qt6OpenGLWidgets", []),
    ("multimedia", "QtMultimedia", "Qt6Multimedia", []),
]

[
    cc_import(
        name = "qt6_%s_linux_import" % name,
        shared_library = "lib/lib%s.so" % library_name,
    )
    for name, include_folder, library_name, _ in QT_LIBRARIES
]

[
    cc_library(
        name = "qt6_%s_linux" % name,
        # When being on Windows this glob will be empty
        hdrs = glob(["%s/**" % include_folder], allow_empty = True),
        includes = ["include"],
        linkopts = ["-l%s" % library_name],
        deps = [":qt6_%s_linux_import" % name],
        # Available from Bazel 4.0.0
        # target_compatible_with = ["@platforms//os:linux"],
    )
    for name, include_folder, library_name, _ in QT_LIBRARIES
]

[
    cc_import(
        name = "qt6_%s_windows_import" % name,
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
        name = "qt6_%s_windows" % name,
        # When being on Linux or macOS this glob will be empty
        hdrs = glob(["include/%s/**" % include_folder], allow_empty = True),
        includes = ["include"],
        # Available from Bazel 4.0.0
        # target_compatible_with = ["@platforms//os:windows"],
        deps = [":qt6_%s_windows_import" % name],
    )
    for name, include_folder, _, _ in QT_LIBRARIES
]

[
    cc_library(
        name = "qt6_%s_osx" % name,
        # When being on Windows or Linux this glob will be empty
        hdrs = glob(["include/%s/**" % include_folder], allow_empty = True),
        includes = ["include"],
        # macOS qt libs do not contain a 6 - e.g. instead of Qt6Core the lib is called QtCore
        linkopts = [
            "-framework %s" % library_name.replace("6", "")
            ],
        # Available from Bazel 4.0.0
        # target_compatible_with = ["@platforms//os:osx"],
    )
    for name, include_folder, library_name, _ in QT_LIBRARIES
]

[
    cc_library(
        name = "qt6_%s" % name,
        visibility = ["//visibility:public"],
        deps = dependencies + select({
            "@platforms//os:linux": [":qt6_%s_linux" % name],
            "@platforms//os:windows": [":qt6_%s_windows" % name],
            "@platforms//os:osx": [":qt6_%s_osx" % name],
        }),
        linkopts = select({
            "@platforms//os:osx": [
              "-F /opt/homebrew/Cellar/qt/6.9.3/Frameworks/",
            ],
            "//conditions:default": [],
        }),
    )
    for name, _, _, dependencies in QT_LIBRARIES
]
