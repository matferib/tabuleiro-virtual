all:
	scons

opengles:
	scons usar_opengl_es=1 gerar_profile=1

windows:
	scons sistema=win32

apple:
	scons sistema=apple

clean:
	scons -c
