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
        name = "qt_%s" % name,
        visibility = ["//visibility:public"],
        deps = dependencies + select({
            "@platforms//os:linux": ["//qt6:qt6_%s" % name],
            "@platforms//os:windows": ["//qt6:qt6_%s" % name],
            "@platforms//os:osx": ["//qt6:qt6_%s" % name],
        }),
    )
    for name, _, _, dependencies in QT_LIBRARIES
]
