.PHONY: all all_sem_testes opengles windows apple linux_profile linux_release clean benchmark benchmark_debug
all_sem_testes:
	scons -j 3

all:
	scons -j 3 testes=1 tabvirt teste_ent_util teste_ent_acoes

release:
	scons -j 3 debug=0

opengles:
	scons -j 3 usar_opengl_es=1 gerar_profile=1

benchmark_es_debug:
	scons -j 3 debug=1 benchmark=1 gerar_profile=1 usar_opengl_es=1 benchmark

benchmark_es:
	scons -j 3 debug=0 benchmark=1 usar_opengl_es=1 benchmark

benchmark_debug:
	scons -j 3 debug=1 benchmark=1 gerar_profile=1 benchmark

benchmark:
	scons -j 3 debug=0 benchmark=1 benchmark

windows:
	scons sistema=win32 debug=0 -j 3

windows_debug:
	scons sistema=win32 debug=1 -j 3

apple:
	scons sistema=apple debug=0 -j 3

apple_profile:
	scons sistema=apple debug=1 gerar_profile=1 -j 3

apple_opengles:
	scons sistema=apple debug=0 usar_opengl_es=1 -j 3

apple_debug:
	scons sistema=apple debug=1 -j 3

apple_debug_opengles:
	scons sistema=apple debug=1 usar_opengl_es=1 -j 3

apple_test:
	scons sistema=apple debug=1 testes=1 teste_ent_ent teste_modelos teste_ent_util teste_arquivo teste_net_util

apple_benchmark:
	scons sistema=apple debug=0 benchmark=1 benchmark

apple_benchmark_debug:
	scons sistema=apple debug=1 benchmark=1 benchmark

linux_profile:
	scons gerar_profile=1 -j 3

linux_release:
	scons debug=0

clean:
	scons -c
