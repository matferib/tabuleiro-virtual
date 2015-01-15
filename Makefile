.PHONY: all all_sem_testes opengles windows apple linux_profile linux_release clean benchmark benchmark_debug
all_sem_testes:
	scons

all:
	scons testes=1

opengles:
	scons usar_opengl_es=1 gerar_profile=1

benchmark_es_debug:
	scons debug=1 benchmark=1 gerar_profile=1 usar_opengl_es=1 benchmark

benchmark_es:
	scons debug=0 benchmark=1 usar_opengl_es=1 benchmark

benchmark_debug:
	scons debug=1 benchmark=1 gerar_profile=1 benchmark

benchmark:
	scons debug=0 benchmark=1 benchmark

windows:
	scons sistema=win32

apple:
	scons sistema=apple debug=0

apple_debug:
	scons sistema=apple debug=1

linux_profile:
	scons gerar_profile=1

linux_release:
	scons debug=0

clean:
	scons -c
