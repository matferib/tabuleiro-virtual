all:
	scons

opengles:
	scons usar_opengl_es=1

clean:
	scons -c
