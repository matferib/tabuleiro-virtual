load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library", "qt_ui_library")

cc_binary(
    name = "tabvirt",
    srcs = ["main.cpp"],
    copts = [
      "-fpic",
    ],
    deps = [
      "@abseil-cpp//absl/strings",
      "@boost//:chrono",
      "@boost//:filesystem",
      "@boost//:system",
      "@boost//:date_time",
      "@boost//:timer",
      "@qt//:qt_core",
      "@qt//:qt_gui",
      "@qt//:qt_opengl",
      "@qt//:qt_network",
      "@qt//:qt_widgets",
      "//ent:ent",
      "//ifg:ifg",
      "//ifg/qt:ifg_qt",
      "//log:log",
      "//m3d:m3d",
      "//net:net",
      "//ntf:ntf",
      "//som:som",
      "//tex:tex",
    ],
    linkopts = select({
      "@platforms//os:osx": [
        "-framework OpenGL",
        "-F /opt/homebrew/Cellar/qt@5/5.15.15/Frameworks/",
        "-framework QtCore",
        "-framework QtGui",
        "-framework QtNetwork",
        "-framework QtOpenGL",
        "-framework QtMultimedia",
        "-framework QtWidgets",
      ],
      "@platforms//os:linux": [
        "-lGLU",
        "-lGL",
      ],
      "//conditions:default": [":generic_lib"],
    })
)
