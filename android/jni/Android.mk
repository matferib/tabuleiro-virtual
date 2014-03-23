LOCAL_PATH := $(call my-dir)

# Protobuffer pre compilado.
# Baseado em file:///opt/android-ndk-r9d/docs/PREBUILTS.html.
include $(CLEAR_VARS)
LOCAL_MODULE := protobuf-prebuilt
LOCAL_SRC_FILES := protobuf-2.5.0/src/.libs/libprotobuf.a
LOCAL_EXPORT_C_INCLUDES := protobuf-2.5.0/src
include $(PREBUILT_STATIC_LIBRARY)

# Boost pre compilado.
# System
include $(CLEAR_VARS)
LOCAL_MODULE := boost-system-prebuilt
LOCAL_SRC_FILES := boost_1_55_0/bin.v2/libs/system/build/gcc-androidR8e/release/link-static/threading-multi/libboost_system-gcc-mt-1_55.a
LOCAL_EXPORT_C_INCLUDES := boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)
# Chrono.
include $(CLEAR_VARS)
LOCAL_MODULE := boost-chrono-prebuilt
LOCAL_SRC_FILES := boost_1_55_0/bin.v2/libs/chrono/build/gcc-androidR8e/release/link-static/threading-multi/libboost_chrono-gcc-mt-1_55.a
LOCAL_EXPORT_C_INCLUDES := boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)
# Timer.
include $(CLEAR_VARS)
LOCAL_MODULE := boost-timer-prebuilt
LOCAL_SRC_FILES := boost_1_55_0/bin.v2/libs/timer/build/gcc-androidR8e/release/link-static/threading-multi/libboost_timer-gcc-mt-1_55.a
LOCAL_EXPORT_C_INCLUDES := boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)



# Tabuleiro.
# Baseado em file:///opt/android-ndk-r9d/docs/ANDROID-MK.html.
include $(CLEAR_VARS)         # Limpa todas variaveis LOCAL_* exceto LOCAL_PATH.
LOCAL_MODULE := tabuleiro     # Modulo definido por este Android.mk.
LOCAL_SRC_FILES := jni-impl.cpp \
	                 gltab/gl.cpp \
									 net/util.cpp net/cliente.cpp \
									 ntf/notificacao.cpp ntf/notificacao.pb.cpp \
									 ent/constantes.cpp ent/entidade.pb.cpp ent/tabuleiro.pb.cpp ent/acoes.pb.cpp ent/entidade.cpp ent/entidade_desenho.cpp ent/tabuleiro.cpp ent/acoes.cpp ent/util.cpp
LOCAL_CPPFLAGS += -DUSAR_OPENGL_ES
LOCAL_STATIC_LIBRARIES := protobuf-prebuilt boost-system-prebuilt boost-timer-prebuilt boost-chrono-prebuilt
LOCAL_LDLIBS := -lGLESv1_CM -llog
LOCAL_CPPFLAGS += -frtti -fexceptions

include $(BUILD_SHARED_LIBRARY)  # Monta biblioteca dinamica libtabuleiro.so.
