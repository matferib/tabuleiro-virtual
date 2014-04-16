# Baseado na documentacao em: file:///opt/android-ndk-r9d/docs/APPLICATION-MK.html.
#APP_MODULES := tabuleiro protobuf-prebuilt boost-prebuilt 
APP_CPPFLAGS := -std=c++11
APP_ABI := armeabi-v7a
APP_PLATFORM := android-14  # 4.0
APP_STL := gnustl_shared
NDK_TOOLCHAIN_VERSION := 4.8
