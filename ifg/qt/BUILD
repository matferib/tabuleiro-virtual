package(default_visibility = ["//visibility:public"])

load("@qt//:qt.bzl", "qt_cc_library")

qt_cc_library(
  name = "ifg_qt",
  srcs = [
    "atualiza_ui.cpp",
    "bonus_util.cpp",
    "constantes.cpp",
    "menuprincipal.cpp",
    "principal.cpp",
    "qt_interface.cpp",
    #"texturas.cpp",
    "util.cpp",
    "visualizador3d.cpp",
    "visualizador3d_dialogos.cpp",
    "som_qt.cpp"
  ],
  normal_hdrs = [],
  hdrs = [
    "atualiza_ui.h",
    "bonus_util.h",
    "constantes.h",
    "evento_util.h",
    "feiticos_util.h",
    "inimigo_predileto_util.h",
    "itens_magicos_util.h",
    "menuprincipal.h",
    "pericias_util.h",
    "principal.h",
    "qt_interface.h",
    "talentos_util.h",
    "texturas.h",
    "util.h",
    "visualizador3d.h",
  ],
  defines = select({
    "@platforms//os:osx": ["USAR_QT5=1"],
    "//conditions:default": ["USAR_QT5=0"],
  }),
  deps = [
    "@abseil-cpp//absl/strings",
    "@abseil-cpp//absl/strings:str_format",
    "@qt//:qt_core",
    "@qt//:qt_gui",
    "@qt//:qt_multimedia",
    "@qt//:qt_opengl",
    "@qt//:qt_openglwidgets",
    "@qt//:qt_widgets",
    "//arq:arq_headers",
    "//ifg:ifg_cc_proto",
    "//ifg/qt/ui:ifg_qt_ui",
    "//ent:ent_headers",
    "//log:log",
    "//m3d:m3d",
    "//net:net",
    "//som:som",
    "//tex:tex",
  ]
)


