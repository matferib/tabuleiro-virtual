LOCAL_PATH := $(call my-dir)

# Qt libs.
ifneq ($(USAR_QT),)
	include $(CLEAR_VARS)
	LOCAL_MODULE := qt5core-prebuilt
	LOCAL_SRC_FILES := ../../../../Qt5/5.10.1/android_$(TARGET_ARCH_ABI)/lib/libQt5Core.so
	include $(PREBUILT_SHARED_LIBRARY)
	include $(CLEAR_VARS)
	LOCAL_MODULE := qt5gui-prebuilt
	LOCAL_SRC_FILES := ../../../../Qt5/5.10.1/android_$(TARGET_ARCH_ABI)/lib/libQt5Gui.so
	include $(PREBUILT_SHARED_LIBRARY)
	include $(CLEAR_VARS)
	LOCAL_MODULE := qt5widgets-prebuilt
	LOCAL_SRC_FILES := ../../../../Qt5/5.10.1/android_$(TARGET_ARCH_ABI)/lib/libQt5Widgets.so
	include $(PREBUILT_SHARED_LIBRARY)
	include $(CLEAR_VARS)
	LOCAL_MODULE := qt5opengl-prebuilt
	LOCAL_SRC_FILES := ../../../../Qt5/5.10.1/android_$(TARGET_ARCH_ABI)/lib/libQt5OpenGL.so
	include $(PREBUILT_SHARED_LIBRARY)
	include $(CLEAR_VARS)
	LOCAL_MODULE := qt5androidplugin-prebuilt
	LOCAL_SRC_FILES := ../../../../Qt5/5.10.1/android_$(TARGET_ARCH_ABI)/plugins/platforms/android/libqtforandroid.so
	include $(PREBUILT_SHARED_LIBRARY)
endif

# Proto: precisa gerar o config.
include $(CLEAR_VARS)
LOCAL_MODULE := protobuf
LOCAL_CPP_FEATURES += rtti
LOCAL_CPPFLAGS += -DHAVE_PTHREAD
LOCAL_SRC_FILES := $(wildcard protobuf-3.0.0/src/google/protobuf/*.cc) \
	                 $(wildcard protobuf-3.0.0/src/google/protobuf/stubs/*.cc) \
	                 $(wildcard protobuf-3.0.0/src/google/protobuf/io/*.cc)
LOCAL_SRC_FILES := $(filter-out %test.cc %unittest.cc %test_util_lite.cc %test_util.cc,$(LOCAL_SRC_FILES))
$(info $$LOCAL_SRC_FILES is [${LOCAL_SRC_FILES}])
LOCAL_C_INCLUDES := protobuf-3.0.0/src protobuf-3.0.0
LOCAL_EXPORT_C_INCLUDES := protobuf-3.0.0/src
include $(BUILD_STATIC_LIBRARY)

# Boost.
include $(CLEAR_VARS)
LOCAL_MODULE := boost
LOCAL_CPP_FEATURES += exceptions
LOCAL_SRC_FILES := $(wildcard boost_1_55_0/libs/filesystem/src/*.cpp) \
	                 $(wildcard boost_1_55_0/libs/system/src/*.cpp) \
									 $(wildcard boost_1_55_0/libs/timer/src/*.cpp) \
									 $(wildcard boost_1_55_0/libs/date_time/src/gregorian/*.cpp) \
									 $(wildcard boost_1_55_0/libs/date_time/src/posix_time/*.cpp) \
									 $(wildcard boost_1_55_0/libs/chrono/src/*.cpp)
LOCAL_C_INCLUDES := boost_1_55_0
LOCAL_EXPORT_C_INCLUDES := boost_1_55_0
include $(BUILD_STATIC_LIBRARY)

# Tabuleiro.
# Baseado em file:///opt/android-ndk-r9d/docs/ANDROID-MK.html.
include $(CLEAR_VARS)         # Limpa todas variaveis LOCAL_* exceto LOCAL_PATH.
LOCAL_MODULE := tabuleiro     # Modulo definido por este Android.mk.
LOCAL_C_INCLUDES += ${ANDROID_NDK}/sources/android/native_app_glue/

ifneq ($(USAR_QT),)
	LOCAL_C_INCLUDES += /home/matheus/Qt5/5.10.1/android_armv7/include/ \
                      /home/matheus/Qt5/5.10.1/android_armv7/include/QtWidgets \
                      /home/matheus/Qt5/5.10.1/android_armv7/include/QtGui \
                      /home/matheus/Qt5/5.10.1/android_armv7/include/QtOpenGL \
                      /home/matheus/Qt5/5.10.1/android_armv7/include/QtCore
endif

LOCAL_SRC_FILES := gltab/gl_es.cpp gltab/gl_comum.cpp gltab/gl_char.cpp gltab/gl_vbo.cpp gltab/glues.cpp \
									 matrix/matrices.cpp \
									 net/util.cpp net/cliente.cpp net/servidor.cpp net/socket.cpp \
									 ntf/notificacao.cpp ntf/notificacao.pb.cpp \
									 ent/constantes.cpp ent/entidade.pb.cpp ent/tabuleiro.pb.cpp ent/acoes.pb.cpp ent/controle_virtual.pb.cpp \
									 ent/entidade.cpp ent/entidade_desenho.cpp ent/entidade_composta.cpp ent/entidade_forma.cpp ent/tabuleiro.cpp \
									 ent/tabuleiro_controle_virtual.cpp ent/tabuleiro_picking.cpp ent/acoes.cpp ent/util.cpp ent/recomputa.cpp \
									 ent/tabuleiro_interface.cpp ent/tabuleiro_tratador.cpp ent/comum.pb.cpp ent/tabelas.cpp ent/tabelas.pb.cpp \
                   ifg/tecladomouse.cpp ifg/interface.cpp ifg/interface_android.cpp ifg/modelos.pb.cc \
                   tex/texturas.cpp tex/lodepng.cpp \
									 m3d/m3d.cpp \
									 som/som.cpp \
									 arq/arquivo.cpp arq/arquivo_android.cpp
ifneq ($(USAR_QT),)
	LOCAL_SRC_FILES += main.cpp \
                     ifg/qt/moc_evento_util.cpp \
                     ifg/qt/util.cpp \
                     ifg/qt/atualiza_ui.cpp \
                     ifg/qt/moc_menuprincipal.cpp \
                     ifg/qt/moc_pericias_util.cpp \
                     ifg/qt/principal.cpp \
                     ifg/qt/visualizador3d.cpp \
                     ifg/qt/menuprincipal.cpp \
                     ifg/qt/moc_principal.cpp \
                     ifg/qt/constantes.cpp \
                     ifg/qt/moc_util.cpp \
                     ifg/qt/qt_interface.cpp
else
	LOCAL_SRC_FILES += jni-impl.cpp
endif

# Flags de performance.
LOCAL_ARM_MODE := arm
#ifneq ($(XOOM),)
#	LOCAL_ARM_NEON := false
#else
#	LOCAL_ARM_NEON := true
#endif

ifneq ($(USAR_QT),)
	LOCAL_CPPFLAGS += -DUSAR_QT -DUSAR_QT5
endif
LOCAL_CPPFLAGS += -DUSAR_OPENGL_ES -DDEBUG=$(DEBUG)
ifneq ($(NEXUS7),)
	LOCAL_CPPFLAGS += -DZBUFFER_16_BITS
endif
ifneq ($(XOOM),)
	LOCAL_CPPFLAGS += -DZBUFFER_16_BITS
endif
LOCAL_STATIC_LIBRARIES := protobuf boost

ifneq ($(USAR_QT),)
	LOCAL_SHARED_LIBRARIES := qt5core-prebuilt qt5gui-prebuilt qt5widgets-prebuilt qt5opengl-prebuilt qt5androidplugin-prebuilt
	LOCAL_STATIC_LIBRARIES += android_native_app_glue
endif
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
ifneq ($(USAR_QT),)
	$(call import-module,android/native_app_glue)
endif
