# Baseado na documentacao em: file:///opt/android-ndk-r9d/docs/APPLICATION-MK.html.
#APP_MODULES := tabuleiro protobuf-prebuilt boost-prebuilt
APP_CPPFLAGS := -std=c++11
APP_ABI := armeabi-v7a
APP_PLATFORM := android-17 # 4.4.2, 21 (5.0) da pau de rede.
APP_STL := gnustl_shared
#NDK_TOOLCHAIN_VERSION := 4.9
NDK_TOOLCHAIN_VERSION := clang
ifeq ($(DEBUG),"1")
APP_OPTIM := debug
else
APP_OPTIM := release
endif
