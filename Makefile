.PHONY: all all_sem_testes opengles windows apple linux_profile linux_release clean benchmark benchmark_debug
all_sem_testes:
	bazel build --config=linux :tabvirt --verbose_failures
	#scons -j 2

util_test:
	 scons -j 2 testes=1 teste_ent_util

all:
	scons -j 2 testes=1 tabvirt teste_ent_util teste_ent_acoes teste_ent_ent teste_modelos teste_ent_util teste_arquivo #teste_net_util

release:
	scons -j 3 debug=0

opengles:
	scons -j 1 usar_opengl_es=1 gerar_profile=1

benchmark_es_debug:
	scons -j 1 debug=1 benchmark=1 gerar_profile=1 usar_opengl_es=1 benchmark

benchmark_es:
	scons -j 1 debug=0 benchmark=1 usar_opengl_es=1 benchmark

benchmark_debug:
	scons -j 1 debug=1 benchmark=1 gerar_profile=1 benchmark

benchmark:
	scons -j 1 debug=0 benchmark=1 benchmark

windows:
	scons sistema=win32 debug=0 -j 1

windows_debug:
	scons sistema=win32 debug=1 -j 1

apple:
	bazel build --config=mac :tabvirt --verbose_failures
	bazel build --config=mac //ent:acoes_test --verbose_failures
	bazel build --config=mac //ent:ent_test --verbose_failures
	bazel build --config=mac //ent:util_test --verbose_failures
	#scons sistema=apple debug=0 -j 2

apple_clean:
	bazel clean --config=mac --verbose_failures

apple_superclean:
	bazel clean --expunge --config=mac --verbose_failures


apple_profile:
	scons sistema=apple debug=1 gerar_profile=1 -j 1

apple_opengles:
	scons sistema=apple debug=0 usar_opengl_es=1 -j 1

apple_debug:
	bazel build --config=mac -c dbg --strip=never :tabvirt --verbose_failures
	bazel build --config=mac -c dbg --strip=never //ent:acoes_test --verbose_failures
	bazel build --config=mac -c dbg --strip=never //ent:ent_test --verbose_failures
	bazel build --config=mac -c dbg --strip=never //ent:util_test --verbose_failures
	
apple_debug_opengles:
	scons sistema=apple debug=1 usar_opengl_es=1 -j 1

apple_test:
	scons -j 2 sistema=apple debug=1 testes=1 teste_ent_acoes teste_ent_ent teste_modelos teste_ent_util teste_arquivo teste_net_util
	#scons sistema=apple debug=1 testes=1 teste_ent_util

apple_benchmark:
	scons sistema=apple debug=0 benchmark=1 benchmark

apple_benchmark_debug:
	scons sistema=apple debug=1 benchmark=1 benchmark

linux_profile:
	scons gerar_profile=1 -j 3

linux_release:
	scons debug=0

clean:
	bazel clean --config=linux
	#scons -c
