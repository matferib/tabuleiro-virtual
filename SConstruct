env = Environment(toolpath=['tools'], tools=['default', 'protoc'])

env['QTDIR'] = '/usr'
env.Tool('qt')

# qt
env['QT_MOCHSUFFIX'] = '.cpp'
env['QT_CPPPATH'] = [env['QTDIR'] + '/include/QtGui', env['QTDIR'] + '/include/QtCore', env['QTDIR'] + '/include', env['QTDIR'] + '/include/QtOpenGL']
env['QT_LIBPATH'] = env['QTDIR'] + '/lib64'
env['QT_LIB'] = ['QtGui', 'QtOpenGL', 'QtCore', 'glut']
env['FRAMEWORKPATH'] = ['/usr/local/lib/']
env['FRAMEWORKS'] = ['QtOpenGL', 'QtGui', 'QtCore', 'OpenGL', 'GLUT']

# protobuffer.
env['PROTOCOUTDIR'] = './'
env['PROTOCPYTHONOUTDIR'] = None

# c++
env['CPPPATH'] += ['./']
env['CPPDEFINES'] = {'USAR_GLOG': 0}
env['CXXFLAGS'] += ['-Wall', '-O2', '-std=c++11']
env['LIBS'] += ['GLU', 'protobuf', 'boost_system', 'boost_timer', 'pthread']

# Configuracoes locais.
env.SConscript('local.SConscript', exports = 'env')

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
ent_proto = env.Protoc(
  target = [],
  source = ['ent/entidade.proto', 'ent/tabuleiro.proto'],
)

# net
cServidor = env.Object('net/servidor.cpp')
cCliente = env.Object('net/cliente.cpp')

# Notificacoes.
cNtf = env.Object('ntf/notificacao.cpp')
ntf_proto = env.Protoc(
  target = [],
  source = ['ntf/notificacao.proto'],
)

# programa final
env.Program(
	target = 'tabvirt',
	source = [
		'main.cpp',
    # net.
    cServidor, cCliente,
		# notificacoes.
		ntf_proto[0], cNtf, 
		# interface QT
		cPrincipal, cMenuPrincipal, cVisualizador3d, cUtil, cTexturas,
    # ent.
		ent_proto[0], ent_proto[2], cTabuleiro, cEntidade
	]
)
