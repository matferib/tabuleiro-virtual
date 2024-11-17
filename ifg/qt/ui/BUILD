package(default_visibility = ["//visibility:public"])

load("//:qt.bzl", "qt_ui_library")

# UI files compilados.
qt_ui_library(
  name = "entidadeui",
  ui = "entidade.ui",
  deps = [],
)
qt_ui_library(
  name = "opcoesui",
  ui = "opcoes.ui",
  deps = [],
)
qt_ui_library(
  name = "cenarioui",
  ui = "cenario.ui",
  deps = [],
)
qt_ui_library(
  name = "formaui",
  ui = "forma.ui",
  deps = [],
)
qt_ui_library(
  name = "listapaginadaui",
  ui = "listapaginada.ui",
  deps = [],
)
qt_ui_library(
  name = "entradastringui",
  ui = "entradastring.ui",
  deps = [],
)
qt_ui_library(
  name = "dialogobonusui",
  ui = "dialogo_bonus.ui",
  deps = [],
)

cc_library(
  name = "ifg_qt_ui",
  srcs = [
  ],
  deps = [
    ":cenarioui",
    ":dialogobonusui",
    ":entidadeui",
    ":entradastringui",
    ":formaui",
    ":listapaginadaui",
    ":opcoesui",
    "@qt//:qt_core",
    "@qt//:qt_widgets",
    "@qt//:qt_opengl",
  ]
)


