LOCAL_PATH := $(call my-dir)

# Proto: precisa gerar o config.
include $(CLEAR_VARS)
LOCAL_MODULE := protobuf
LOCAL_CPP_FEATURES += rtti
LOCAL_SRC_FILES := $(wildcard protobuf-2.6.1/src/google/protobuf/*.cc) \
	                 $(wildcard protobuf-2.6.1/src/google/protobuf/stubs/*.cc) \
	                 $(wildcard protobuf-2.6.1/src/google/protobuf/io/*.cc)
LOCAL_SRC_FILES := $(filter-out %unittest.cc %test_util_lite.cc %test_util.cc,$(LOCAL_SRC_FILES))
$(info $$LOCAL_SRC_FILES is [${LOCAL_SRC_FILES}])
LOCAL_C_INCLUDES := protobuf-2.6.1/src protobuf-2.6.1
LOCAL_EXPORT_C_INCLUDES := protobuf-2.6.1/src
include $(BUILD_STATIC_LIBRARY)

# Boost.
include $(CLEAR_VARS)
LOCAL_MODULE := boost
LOCAL_CPP_FEATURES += exceptions
LOCAL_SRC_FILES := $(wildcard boost_1_55_0/libs/filesystem/src/*.cpp) \
	                 $(wildcard boost_1_55_0/libs/system/src/*.cpp) \
									 $(wildcard boost_1_55_0/libs/timer/src/*.cpp) \
									 $(wildcard boost_1_55_0/libs/chrono/src/*.cpp)
LOCAL_C_INCLUDES := boost_1_55_0
LOCAL_EXPORT_C_INCLUDES := boost_1_55_0
include $(BUILD_STATIC_LIBRARY)

# Tabuleiro.
# Baseado em file:///opt/android-ndk-r9d/docs/ANDROID-MK.html.
include $(CLEAR_VARS)         # Limpa todas variaveis LOCAL_* exceto LOCAL_PATH.
LOCAL_MODULE := tabuleiro     # Modulo definido por este Android.mk.
LOCAL_SRC_FILES := jni-impl.cpp \
	                 gltab/gl_es.cpp gltab/gl_comum.cpp gltab/gl_char.cpp gltab/gl_vbo.cpp gltab/glues.cpp \
									 matrix/matrices.cpp \
									 net/util.cpp net/cliente.cpp net/servidor.cpp net/socket.cpp \
									 ntf/notificacao.cpp ntf/notificacao.pb.cpp \
									 ent/constantes.cpp ent/entidade.pb.cpp ent/tabuleiro.pb.cpp ent/acoes.pb.cpp ent/controle_virtual.pb.cpp ent/entidade.cpp ent/entidade_desenho.cpp ent/entidade_composta.cpp ent/entidade_forma.cpp ent/tabuleiro.cpp ent/tabuleiro_controle_virtual.cpp ent/tabuleiro_picking.cpp ent/acoes.cpp ent/util.cpp ent/tabuleiro_interface.cpp \
                   ifg/tecladomouse.cpp ifg/interface.cpp ifg/interface_android.cpp ifg/modelos.pb.cc \
                   tex/texturas.cpp tex/lodepng.cpp \
									 m3d/m3d.cpp \
									 arq/arquivo.cpp arq/arquivo_android.cpp

# Flags de performance.
LOCAL_ARM_MODE := arm
ifneq ($(XOOM),)
	LOCAL_ARM_NEON := false
else
	LOCAL_ARM_NEON := true
endif

LOCAL_CPPFLAGS += -DUSAR_OPENGL_ES -DDEBUG=$(DEBUG)
ifneq ($(NEXUS7),)
	LOCAL_CPPFLAGS += -DZBUFFER_16_BITS
endif
ifneq ($(XOOM),)
	LOCAL_CPPFLAGS += -DZBUFFER_16_BITS
endif
LOCAL_STATIC_LIBRARIES := protobuf boost
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -landroid
LOCAL_CPP_FEATURES := rtti exceptions

ifneq ($(PROFILER_LIGADO),)
	LOCAL_CFLAGS := -pg -DPROFILER_LIGADO -Iandroid-ndk-profiler
	LOCAL_LDFLAGS := -pg
	LOCAL_STATIC_LIBRARIES += android-ndk-profiler
endif

include $(BUILD_SHARED_LIBRARY)  # Monta biblioteca dinamica libtabuleiro.so.
ifneq ($(PROFILER_LIGADO),)
  $(call import-module,android-ndk-profiler)
endif
