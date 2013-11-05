env = Environment(toolpath=['tools'], tools=['default', 'protoc'])

env['QTDIR'] = '/usr'
env.Tool('qt')

# qt
env['QT_LIB'] = 'QtGui'
env['QT_MOCHSUFFIX'] = '.cpp'
env['QT_CPPPATH'] = [env['QTDIR'] + '/include/QtGui', env['QTDIR'] + '/include/QtCore', env['QTDIR'] + '/include', env['QTDIR'] + '/include/QtOpenGL']
env['QT_LIBPATH'] = env['QTDIR'] + '/lib64'
env['QT_LIB'] = ['QtGui', 'QtOpenGL', 'QtCore', 'glut']

# protobuffer.
env['PROTOCOUTDIR'] = './'
env['PROTOCPYTHONOUTDIR'] = None

# c++
env['CPPPATH'] += ['./']
env['CXXFLAGS'] += ['-g', '-Wall', '-std=c++11']
env['LIBS'] += ['GLU', 'protobuf']


# Principal: qt moc e fonte. Os mocs sao gerados automaticamente
# se estiverem no mesmo diretorio do fonte.
cPrincipal = env.Object('ifg/qt/principal.cpp')

# MenuPrincipal: qt moc e fonte
cMenuPrincipal = env.Object('ifg/qt/menuprincipal.cpp')

# visualizador3d: qt moc e fonte
cVisualizador3d = env.Object('ifg/qt/visualizador3d.cpp')

# ent
cTabuleiro = env.Object('ent/tabuleiro.cpp')
cEntidade = env.Object('ent/entidade.cpp')
cParametrosDesenho = env.Object('ent/parametrosdesenho.cpp')
cMovel = env.Object('ent/movel.cpp')

# Notificacoes.
ntf = env.Object('ntf/notificacao.cpp')
ntf_proto = env.Protoc(
  target = [],
  source = ['ntf/notificacao.proto'],
)

# programa final
env.Program(
	target = 'tabvirt',
	source = [
		'main.cpp', 
		# notificacoes.
		ntf_proto[0], ntf, 
		# interface QT
		cPrincipal, cMenuPrincipal, cVisualizador3d, 
		cTabuleiro, cEntidade, cParametrosDesenho, cMovel
	]
)




