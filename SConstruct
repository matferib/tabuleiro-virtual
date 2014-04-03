import os
import sys

env = Environment(ENV=os.environ, toolpath=['tools'], tools=['protoc'])

sistema = ARGUMENTS.get('sistema', 'linux')
if sistema not in ['win32', 'linux', 'apple']:
  sys.exit('Sistema invalido: ' + sistema)
print 'Usando sistema: ' + sistema

env.Tool('qt')
if sistema == 'win32':
  env.Tool('mingw')
else:
  env.Tool('default')

# qt
env['QT_MOCHSUFFIX'] = '.cpp'
if sistema == 'win32':
  env['QTDIR'] = 'c:/Qt/4.8.5/'
  env['QT_LIBPATH'] = 'win32/lib'
  env['QT_LIB'] = ['QtOpenGL', 'QtGui', 'QtCore']
elif sistema == 'apple':
  env['FRAMEWORKPATH'] = ['/usr/local/lib/']
  env['FRAMEWORKS'] = ['QtOpenGL', 'QtGui', 'QtCore', 'OpenGL', 'GLUT']
  env['QT_LIB'] = []
else:
  env['QTDIR'] = '/usr'
  env['QT_LIBPATH'] = env['QTDIR'] + '/lib64'
  env['QT_LIB'] = ['QtGui', 'QtOpenGL', 'QtCore']
  env['QT_CPPPATH'] = [env['QTDIR'] + '/include/QtGui', env['QTDIR'] + '/include/QtCore', env['QTDIR'] + '/include', env['QTDIR'] + '/include/QtOpenGL']

# protobuffer.
env['PROTOCOUTDIR'] = './'
env['PROTOCPYTHONOUTDIR'] = None

# c++
if sistema == 'win32':
  env['CPPPATH'] += ['./', 'win32/include', 'c:/Users/Matheus/Downloads/boost_1_55_0/']
  env['CPPDEFINES'] = {'USAR_GLOG': 0, 'WIN32_LEAN_AND_MEAN': 1, 'WIN32': 1, '_WINDOWS': 1, '_CRT_SECURE_NO_WARNINGS': 1, '_WIN32_WINNT': 0x0601, 'WINVER': 0x0601, 'FREEGLUT_STATIC': 1, 'QT_STATIC_BUILD': 1}
  env['CXXFLAGS'] += ['-std=c++11', '-Wall', '-O2', '-Wfatal-errors']
  env['LIBS'] += ['freeglut_static', 'glu32', 'opengl32', 'protobuf', 'boost_system-mgw48-mt-1_55', 'boost_timer-mgw48-mt-1_55', 'boost_chrono-mgw48-mt-1_55', 'ws2_32', 'Mswsock', 'Gdi32', 'Winmm', 'ole32', 'Oleacc', 'OleAut32', 'libuuid', 'Comdlg32', 'imm32', 'Winspool']
  env['LIBPATH'] += [ 'win32/lib' ]
  env['LINKFLAGS'] += ['-Wl,--subsystem,windows']
elif sistema == 'apple':
  env['CPPPATH'] += ['./',
                     '/usr/local/lib/QtGui.framework/Headers',
                     '/usr/local/lib/QtOpenGL.framework/Headers',
                     '/usr/local/lib/QtCore.framework/Headers']
  env['CPPDEFINES'] = {'USAR_GLOG': 0}
  env['CXXFLAGS'] += ['-Wall', '-O2', '-std=c++11']
  env['LIBS'] += ['protobuf', 'boost_system', 'boost_timer', 'pthread']
else:
  env['CPPPATH'] += ['./', ]
  env['CPPDEFINES'] = {'USAR_GLOG': 0, 'USAR_WATCHDOG': 1}
  env['CXXFLAGS'] = ['-Wall', '-g', '-std=c++11']
  env['LIBS'] += ['glut', 'GLU', 'protobuf', 'boost_system', 'boost_timer', 'pthread']

usar_opengl_es = (ARGUMENTS.get('usar_opengl_es', 0) == '1')
print 'usar_opengl_es : %r' % usar_opengl_es
if usar_opengl_es:
  env['CPPPATH'] += ['./opengl_es/']
  env['CPPDEFINES']['USAR_OPENGL_ES'] = 1

gerar_profile = (ARGUMENTS.get('gerar_profile', 0) == '1')
if gerar_profile:
  env['CXXFLAGS'] += ['-pg']
  env['LINKFLAGS'] += ['-pg']

# Configuracoes locais.
env.SConscript('local.SConscript', exports = 'env')

# Daqui pra baixo eh tudo comum.

# Permite lambdas no QT.
cUtil = env.Object('ifg/qt/util.cpp')

# Principal: qt moc e fonte. Os mocs sao gerados automaticamente
# se estiverem no mesmo diretorio do fonte.
cPrincipal = env.Object('ifg/qt/principal.cpp')

# MenuPrincipal: qt moc e fonte
cMenuPrincipal = env.Object('ifg/qt/menuprincipal.cpp')

# visualizador3d: qt moc e fonte
cVisualizador3d = env.Object('ifg/qt/visualizador3d.cpp')

# Implementacao das texturas.
cTexturas = env.Object('ifg/qt/texturas.cpp')

# ent
cTabuleiro = env.Object('ent/tabuleiro.cpp')
cEntidade = env.Object('ent/entidade.cpp')
cAcoes = env.Object('ent/acoes.cpp')
cConstantes = env.Object('ent/constantes.cpp')
cEntUtil = env.Object('ent/util.cpp')
cEntDesenho = env.Object('ent/entidade_desenho.cpp')
if sistema == 'linux':
  cEntWatchdog = env.Object('ent/watchdog.cpp')

ent_proto = env.Protoc(
  target = [],
  source = ['ent/entidade.proto', 'ent/tabuleiro.proto', 'ent/acoes.proto'],
)

# net
cNetServidor = env.Object('net/servidor.cpp')
cNetCliente = env.Object('net/cliente.cpp')
cNetUtil = env.Object('net/util.cpp')

# Notificacoes.
cNtf = env.Object('ntf/notificacao.cpp')
ntf_proto = env.Protoc(
  target = [],
  source = ['ntf/notificacao.proto'],
)

# GL
cGl = env.Object('gltab/gl.cpp')

# programa final
env.Program(
	target = 'tabvirt',
	source = [
		'main.cpp',
    # net.
    cNetServidor, cNetCliente, cNetUtil,
		# notificacoes.
		ntf_proto[0], cNtf,
		# interface QT
		cPrincipal, cMenuPrincipal, cVisualizador3d, cUtil, cTexturas,
    # ent. Os protos sao de 2 em 2 para nao incluir os cabecalhos.
		ent_proto[0], ent_proto[2], ent_proto[4], cTabuleiro, cEntidade, cAcoes, cConstantes, cEntUtil, cEntDesenho,
    # gl.
    cGl,
	] + ([ cEntWatchdog ] if sistema == 'linux' else [])
)
