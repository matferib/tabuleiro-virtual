LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := tabuleiro

LOCAL_CPPFLAGS += -std=c++11 -DANDROID_NDK -DUSAR_OPENGL_ES
LOCAL_CFLAGS := -std=c++11 -DANDROID_NDK -DUSAR_OPENGL_ES

LOCAL_SRC_FILES := jni-impl.cpp gl/gl.cpp ent/constantes.cpp net/util.cpp net/cliente.cpp

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

include $(BUILD_SHARED_LIBRARY)
