cc_binary(
    name = "tabvirt",
    srcs = ["main.cpp"],
    cxxopts = [
      "-fpic",
      "-Wno-deprecated-declarations",
    ],
    deps = [
      "@abseil-cpp//absl/strings",
      "@boost//:chrono",
      "@boost//:filesystem",
      "@boost//:system",
      "@boost//:date_time",
      "@boost//:timer",
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
        "-F /opt/homebrew/Cellar/qt@5/5.15.15/Frameworks",
      ],
      "@platforms//os:linux": [
        "-lGLU",
        "-lGL",
      ],
      "//conditions:default": [":generic_lib"],
    })
)

