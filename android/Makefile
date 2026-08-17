all:
	bazel build --config=android //src/com/matferib/Tabuleiro:tabvirt --verbose_failures

install:
	bazel mobile-install --config=android //src/com/matferib/Tabuleiro:tabvirt --adb=${ANDROID_SDK}/platform-tools/adb
