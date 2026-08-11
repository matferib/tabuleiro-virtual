load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "gltab_headers",
  hdrs = [
    "gl.h",
    "gl_interno.h",
    "glues.h",
    "gl_vbo.h",
  ],
  defines = ["GL_GLEXT_PROTOTYPES=1", "USAR_OPENGL_ES=1"],
  deps = [
  ],
  cxxopts = [
    "-Ijni",
  ],
)

cc_library(
  name = "gltab",
  hdrs = [
    "gl.h",
    "gl_interno.h",
    "glues.h",
    "gl_vbo.h",
  ],
  srcs = [
      "gl_char.cpp",
      "gl_comum.cpp",
      "gl_es.cpp",
      "gl_vbo.cpp",
      "glues.cpp",
  ],
  defines = ["GL_GLEXT_PROTOTYPES=1", "USAR_OPENGL_ES=1"],
  deps = [
      "@abseil-cpp//absl/strings",
      "@abseil-cpp//absl/strings:str_format",
      "//jni/arq",
      "//jni/log",
      "//jni/matrix",
  ],
  cxxopts = [
    "-Ijni",
  ],
)
