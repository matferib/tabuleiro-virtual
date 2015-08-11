# Baseado na documentacao em: file:///opt/android-ndk-r9d/docs/APPLICATION-MK.html.
#APP_MODULES := tabuleiro protobuf-prebuilt boost-prebuilt
APP_CPPFLAGS := -std=c++11
APP_ABI := armeabi-v7a
APP_PLATFORM := android-19  # 4.4 
APP_STL := gnustl_shared
NDK_TOOLCHAIN_VERSION := 4.9
APP_OPTIM := release
