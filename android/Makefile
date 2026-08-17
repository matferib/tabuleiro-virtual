all:
	bazel build --config=android //src/com/matferib/Tabuleiro:tabvirt --verbose_failures

install:
	bazel build --config=android //src/com/matferib/Tabuleiro:tabvirt --verbose_failures
	bazel mobile-install --config=android //src/com/matferib/Tabuleiro:tabvirt --adb=${ANDROID_SDK}/platform-tools/adb

install_debug:
	bazel build --config=android -c dbg //src/com/matferib/Tabuleiro:tabvirt --verbose_failures
	bazel mobile-install --config=android -c dbg --start=DEBUG //src/com/matferib/Tabuleiro:tabvirt --adb=${ANDROID_SDK}/platform-tools/adb
