.PHONY: all all_sem_testes opengles windows apple linux_profile linux_release clean benchmark benchmark_debug
tabuleiro:
	bazel build --cxxopt=-DNDEBUG --cxxopt=-O2 --copt=-O2 --config=linux :tabvirt --verbose_failures

debug:
	bazel build -c dbg --config=linux :tabvirt --verbose_failures

all:
	bazel build --cxxopt=-DNDEBUG --cxxopt=-O2 --config=linux :tabvirt --verbose_failures
	bazel build --cxxopt=-DNDEBUG --cxxopt=-O2 --config=linux //ent:acoes_test --verbose_failures
	bazel build --cxxopt=-DNDEBUG --cxxopt=-O2 --config=linux //ent:ent_test --verbose_failures
	bazel build --cxxopt=-DNDEBUG --cxxopt=-O2 --config=linux //ent:util_test --verbose_failures

clean:
	bazel clean --config=linux

superclean:
	bazel clean --expunge --config=linux --verbose_failures

opengles:
	scons -j 1 usar_opengl_es=1 gerar_profile=1

windows:
	scons sistema=win32 debug=0 -j 1

windows_debug:
	scons sistema=win32 debug=1 -j 1

apple:
	bazel build -c opt --config=mac :tabvirt --verbose_failures
	bazel build -c opt --config=mac //ent:acoes_test --verbose_failures
	bazel build -c opt --config=mac //ent:ent_test --verbose_failures
	bazel build -c opt --config=mac //ent:util_test --verbose_failures

apple_clean:
	bazel clean --config=mac --verbose_failures

apple_superclean:
	bazel clean --expunge --config=mac --verbose_failures


apple_opengles:
	scons sistema=apple debug=0 usar_opengl_es=1 -j 1

apple_debug:
	bazel build --config=mac -c dbg --strip=never :tabvirt --verbose_failures
	bazel build --config=mac -c dbg --strip=never //ent:acoes_test --verbose_failures
	bazel build --config=mac -c dbg --strip=never //ent:ent_test --verbose_failures
	bazel build --config=mac -c dbg --strip=never //ent:util_test --verbose_failures
	
