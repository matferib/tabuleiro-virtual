package(default_visibility = ["//visibility:public"])

proto_library(
    name = "acoes_proto",
    srcs = ["acoes.proto"],
    deps = [":comum_proto"],
)

proto_library(
    name = "comum_proto",
    srcs = ["comum.proto"],
)

proto_library(
    name = "controle_virtual_proto",
    srcs = ["controle_virtual.proto"],
    deps = [":comum_proto"],
)

proto_library(
    name = "entidade_proto",
    srcs = ["entidade.proto"],
    deps = [
        ":acoes_proto",
        ":comum_proto",
    ],
)

proto_library(
    name = "geracao_proto",
    srcs = ["geraco.proto"],
)

proto_library(
    name = "tabelas_proto",
    srcs = ["tabelas.proto"],
    deps = [
        ":acoes_proto",
        ":comum_proto",
    ],
)

proto_library(
    name = "tabuleiro_proto",
    srcs = ["tabuleiro.proto"],
    deps = [
        ":comum_proto",
        ":entidade_proto",
    ],
)

proto_library(
    name = "tabuleiro_interface_proto",
    srcs = ["tabuleiro_interface.proto"],
)

cc_proto_library(
    name = "acoes_cc_proto",
    deps = ["acoes_proto"],
)

cc_proto_library(
    name = "comum_cc_proto",
    deps = ["comum_proto"],
)

cc_proto_library(
    name = "controle_virtual_cc_proto",
    deps = ["controle_virtual_proto"],
)

cc_proto_library(
    name = "entidade_cc_proto",
    deps = ["entidade_proto"],
)

cc_proto_library(
    name = "geracao_cc_proto",
    deps = ["geraco_proto"],
)

cc_proto_library(
    name = "tabelas_cc_proto",
    deps = ["tabelas_proto"],
)

cc_proto_library(
    name = "tabuleiro_cc_proto",
    deps = ["tabuleiro_proto"],
)

cc_proto_library(
    name = "tabuleiro_interface_cc_proto",
    deps = ["tabuleiro_interface_proto"],
)

cc_library(
    name = "ent_headers",
    hdrs = [
        "acoes.h",
        "constantes.h",
        "entidade.h",
        "recomputa.h",
        "tabelas.h",
        "tabuleiro.h",
        "tabuleiro_interface.h",
        "tabuleiro_terreno.h",
        "util.h",
        "watchdog.h",
    ],
    deps = [
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/strings",
        "@boost//:chrono",
        "@boost//:filesystem",
        "@boost//:system",
        "@boost//:date_time",
        "@boost//:timer",
        ":acoes_cc_proto",
        ":controle_virtual_cc_proto",
        ":entidade_cc_proto",
        ":tabelas_cc_proto",
        ":tabuleiro_cc_proto",
        "//gltab:gltab_headers",
        "//ifg:ifg_headers",
        "//matrix",
        "//ntf:ntf_headers",
    ],
)

cc_library(
    name = "ent",
    srcs = [
        "acoes.cpp",
        "constantes.cpp",
        "entidade.cpp",
        "entidade_composta.cpp",
        "entidade_desenho.cpp",
        "entidade_forma.cpp",
        "recomputa.cpp",
        "tabelas.cpp",
        "tabuleiro.cpp",
        "tabuleiro_controle_virtual.cpp",
        "tabuleiro_interface.cpp",
        "tabuleiro_picking.cpp",
        "tabuleiro_tratador.cpp",
        "util.cpp",
        "watchdog.cpp",
    ],
    hdrs = [
        "acoes.h",
        "constantes.h",
        "entidade.h",
        "recomputa.h",
        "tabelas.h",
        "tabuleiro.h",
        "tabuleiro_interface.h",
        "tabuleiro_terreno.h",
        "util.h",
        "watchdog.h",
    ],
    deps = [
        ":acoes_cc_proto",
        ":controle_virtual_cc_proto",
        ":entidade_cc_proto",
        ":tabelas_cc_proto",
        ":tabuleiro_cc_proto",
        "//arq",
        "//gltab",
        "//ifg",
        "//ifg:ifg_cc_proto",
        "//m3d",
        "//matrix",
        "//net",
        "//ntf",
        "//som",
        "//tex",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/strings",
        "@boost//:chrono",
        "@boost//:filesystem",
        "@boost//:system",
        "@boost//:date_time",
        "@boost//:timer",
    ],
)

cc_test(
    name = "ent_test",
    srcs = ["ent_test.cpp"],
    deps = [
        ":ent",
        "//som:som_dummy",  # para nao linkaer QT.
        "@googletest//:gtest",
        "@googletest//:gtest_main",
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

cc_test(
    name = "acoes_test",
    srcs = ["acoes_test.cpp"],
    deps = [
        ":ent",
        "//som:som_dummy",  # para nao linkar QT.
        "@googletest//:gtest",
        "@googletest//:gtest_main",
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

cc_test(
    name = "util_test",
    srcs = ["util_test.cpp"],
    deps = [
        ":ent",
        "//som:som_dummy",  # para nao linkar QT.
        "@googletest//:gtest",
        "@googletest//:gtest_main",
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
