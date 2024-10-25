package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ifg_qt",
  srcs = [
    "atualiza_ui.cpp",
    "constantes.cpp",
    "menuprincipal.cpp",
    "principal.cpp",
    "qt_interface.cpp",
    #"texturas.cpp",
    "util.cpp",
    "visualizador3d.cpp",
    "visualizador3d_dialogos.cpp",
  ] + glob(["moc*.cpp"]),
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
  copts = [
    "-DUSAR_QT5=1",
  ],
  deps = [
      "@abseil-cpp//absl/strings",
      "@abseil-cpp//absl/strings:str_format",
      "//arq:arq_headers",
      "//ifg:ifg_cc_proto",
      "//ifg/qt/ui:ifg_qt_ui",
      "//ent:ent_headers",
      "//log:log",
      "//m3d:m3d",
      "//net:net",
      "//tex:tex",
  ]
)


