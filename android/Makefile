install:
	bazel build --config=android //src/com/matferib/Tabuleiro:tabvirt --define=DEBUG=0 --verbose_failures
	bazel mobile-install --config=android //src/com/matferib/Tabuleiro:tabvirt --adb=${ANDROID_SDK}/platform-tools/adb

install_debug:
	bazel build --config=android -c dbg --define=DEBUG=1 //src/com/matferib/Tabuleiro:tabvirt --verbose_failures
	bazel mobile-install --config=android -c dbg //src/com/matferib/Tabuleiro:tabvirt --adb=${ANDROID_SDK}/platform-tools/adb --action_env=FORCE_REBUILD=$(date +%s)

install_debugger:
	bazel build --config=android -c dbg --define=DEBUG=1 //src/com/matferib/Tabuleiro:tabvirt --verbose_failures
	bazel mobile-install --config=android -c dbg --start=DEBUG //src/com/matferib/Tabuleiro:tabvirt --adb=${ANDROID_SDK}/platform-tools/adb
