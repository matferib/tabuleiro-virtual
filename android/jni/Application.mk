# Baseado na documentacao em: file:///opt/android-ndk-r9d/docs/APPLICATION-MK.html.
#APP_MODULES := tabuleiro protobuf-prebuilt boost-prebuilt
APP_CPPFLAGS := -std=c++17 -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR

ifneq ($(USAR_QT),)
	APP_ABI := armeabi-v7a arm64-v8a
else
	APP_ABI := armeabi-v7a arm64-v8a
endif
APP_PLATFORM := android-29 # 4.4.2, 21 (5.0) da pau de rede.
APP_STL := c++_shared
NDK_TOOLCHAIN_VERSION := clang

ifeq ($(DEBUG),"1")
APP_OPTIM := debug
else
APP_OPTIM := release
endif
