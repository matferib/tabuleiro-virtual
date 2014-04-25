.PHONY: all all_sem_testes opengles windows apple linux_profile linux_release clean benchmark
all_sem_testes:
	scons

all:
	scons testes=1

opengles:
	scons usar_opengl_es=1 gerar_profile=1

benchmark:
	scons benchmark=1 usar_opengl_es=1 gerar_profile=1

windows:
	scons sistema=win32

apple:
	scons sistema=apple

linux_profile:
	scons gerar_profile=1

linux_release:
	scons debug=0

clean:
	scons -c
