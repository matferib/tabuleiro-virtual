LOCAL_PATH := $(call my-dir)

# Protobuffer pre compilado.
# Baseado em file:///opt/android-ndk-r9d/docs/PREBUILTS.html.
include $(CLEAR_VARS)
LOCAL_MODULE := protobuf-prebuilt
LOCAL_SRC_FILES := protobuf-2.6.1/src/.libs/libprotobuf.a
LOCAL_EXPORT_C_INCLUDES := protobuf-2.6.1/src
include $(PREBUILT_STATIC_LIBRARY)

# Boost pre compilado.
# System
include $(CLEAR_VARS)
LOCAL_MODULE := boost-system-prebuilt
LOCAL_SRC_FILES := boost_1_55_0/bin.v2/libs/system/build/gcc-androidR8e/release/link-static/target-os-linux/threading-multi/libboost_system-gcc-mt-1_55.a
LOCAL_EXPORT_C_INCLUDES := boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)
# Chrono.
include $(CLEAR_VARS)
LOCAL_MODULE := boost-chrono-prebuilt
LOCAL_SRC_FILES := boost_1_55_0/bin.v2/libs/chrono/build/gcc-androidR8e/release/link-static/target-os-linux/threading-multi/libboost_chrono-gcc-mt-1_55.a
LOCAL_EXPORT_C_INCLUDES := boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)
# Timer.
include $(CLEAR_VARS)
LOCAL_MODULE := boost-timer-prebuilt
LOCAL_SRC_FILES := boost_1_55_0/bin.v2/libs/timer/build/gcc-androidR8e/release/link-static/target-os-linux/threading-multi/libboost_timer-gcc-mt-1_55.a
LOCAL_EXPORT_C_INCLUDES := boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)
# Filesystem.
include $(CLEAR_VARS)
LOCAL_MODULE := boost-filesystem-prebuilt
LOCAL_SRC_FILES := boost_1_55_0/bin.v2/libs/filesystem/build/gcc-androidR8e/release/link-static/target-os-linux/threading-multi/libboost_filesystem-gcc-mt-1_55.a
LOCAL_EXPORT_C_INCLUDES := boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)



# Tabuleiro.
# Baseado em file:///opt/android-ndk-r9d/docs/ANDROID-MK.html.
include $(CLEAR_VARS)         # Limpa todas variaveis LOCAL_* exceto LOCAL_PATH.
LOCAL_MODULE := tabuleiro     # Modulo definido por este Android.mk.
LOCAL_SRC_FILES := jni-impl.cpp \
	                 gltab/gl_es.cpp gltab/gl_comum.cpp gltab/gl_char.cpp gltab/gl_vbo.cpp \
									 net/util.cpp net/cliente.cpp net/socket.cpp \
									 ntf/notificacao.cpp ntf/notificacao.pb.cpp \
									 ent/constantes.cpp ent/entidade.pb.cpp ent/tabuleiro.pb.cpp ent/acoes.pb.cpp ent/entidade.cpp ent/entidade_desenho.cpp ent/entidade_composta.cpp ent/entidade_forma.cpp ent/tabuleiro.cpp ent/tabuleiro_controle_virtual.cpp ent/tabuleiro_picking.cpp ent/acoes.cpp ent/util.cpp \
                   ifg/tecladomouse.cpp \
                   tex/texturas.cpp tex/lodepng.cpp \
									 m3d/m3d.cpp \
									 arq/arquivo.cpp arq/arquivo_android.cpp

# Flags de performance.
LOCAL_ARM_MODE := arm
LOCAL_ARM_NEON := true

LOCAL_CPPFLAGS += -DUSAR_OPENGL_ES
ifneq ($(NEXUS7),)
	LOCAL_CPPFLAGS += -DZBUFFER_16_BITS
endif
LOCAL_STATIC_LIBRARIES := protobuf-prebuilt boost-system-prebuilt boost-timer-prebuilt boost-chrono-prebuilt boost-filesystem-prebuilt
LOCAL_LDLIBS := -lGLESv1_CM -llog -landroid
LOCAL_CPP_FEATURES := rtti exceptions

ifneq ($(PROFILER_LIGADO),)
	LOCAL_CFLAGS := -pg -DPROFILER_LIGADO
	LOCAL_STATIC_LIBRARIES += android-ndk-profiler
endif

include $(BUILD_SHARED_LIBRARY)  # Monta biblioteca dinamica libtabuleiro.so.
ifneq ($(PROFILER_LIGADO),)
	$(call import-module,android-ndk-profiler)
endif
