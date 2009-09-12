env = Environment()

env['QTDIR'] = '/opt/qt-4.5.2/'
env.Tool('qt')

# qt
env['QT_LIB'] = 'QtGui'
env['QT_MOCHSUFFIX'] = '.cpp'
env['QT_CPPPATH'] = [env['QTDIR'] + '/include/QtGui', env['QTDIR'] + '/include', env['QTDIR'] + '/include/QtOpenGL']
env['QT_LIBPATH'] = env['QTDIR'] + '/lib'
env['QT_LIB'] = ['QtGui', 'QtOpenGL', 'glut']

# c++
env['CPPPATH'] += ['include']
env['CXXFLAGS'] += ['-g', '-Wall']

# Principal: qt moc e fonte
hPrincipal = env.Moc('include/ifg/qt/principal.h')
cPrincipal = env.Object('ifg/qt/principal.cpp')

# MenuPrincipal: qt moc e fonte
hMenuPrincipal = env.Moc('include/ifg/qt/menuprincipal.h')
cMenuPrincipal = env.Object('ifg/qt/menuprincipal.cpp')

# visualizador3d: qt moc e fonte
#hVisualizador3d = env.Moc('include/ifg/qt/visualizador3d.h')
cVisualizador3d = env.Object('ifg/qt/visualizador3d.cpp')

# ent
cTabuleiro = env.Object('ent/tabuleiro.cpp')
cParametrosDesenho = env.Object('ent/parametrosdesenho.cpp')
cMovel = env.Object('ent/movel.cpp')


# programa final
env.Program(
	target = 'tabvirt',
	source = [
		'main.cpp',
		# notificacoes
		'ntf/notificacao.cpp',
		# interface QT
		hPrincipal, cPrincipal, hMenuPrincipal, cMenuPrincipal, cVisualizador3d, 
		cTabuleiro, cParametrosDesenho, cMovel
	]
)




