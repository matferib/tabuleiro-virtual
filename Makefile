all:
	scons

windows:
	scons sistema=win32

apple:
	scons sistema=apple

clean:
	scons -c
