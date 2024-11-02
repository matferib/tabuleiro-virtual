package(default_visibility = ["//visibility:public"])

cc_library(
  name = "gltab_headers",
  hdrs = [
    "gl.h",
    "gl_interno.h",
    "glues.h",
    "gl_vbo.h",
  ],
  defines = ["GL_GLEXT_PROTOTYPES=1"],
  deps = [
  ]
)

cc_library(
  name = "gltab",
  hdrs = [
    "gl.h",
    "gl_interno.h",
    "glues.h",
    "gl_vbo.h",
    "ext/GL/glext.h",
    "ext/KHR/khrplatform.h",
  ],
  srcs = [
      "gl_char.cpp",
      "gl_comum.cpp",
      "gl_normal.cpp",
      "gl_vbo.cpp",
      "glues.cpp",
  ],
  defines = ["GL_GLEXT_PROTOTYPES=1"],
  includes = ["ext"],
  deps = [
      "@abseil-cpp//absl/strings",
      "@abseil-cpp//absl/strings:str_format",
      "//arq",
      "//log",
      "//matrix",
  ]
)
