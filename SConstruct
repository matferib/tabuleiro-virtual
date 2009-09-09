env = Environment()

env['QTDIR'] = '/opt/qt-4.5.2/'
env.Tool('qt')

# qt
env['QT_LIB'] = 'QtGui'
env['QT_MOCHSUFFIX'] = '.cpp'
env['QT_CPPPATH'] = [env['QTDIR'] + '/include/QtGui', env['QTDIR'] + '/include', env['QTDIR'] + '/include/QtOpenGL']
env['QT_LIBPATH'] = env['QTDIR'] + '/lib'
env['QT_LIB'] = ['QtGui', 'QtOpenGL']

# c++
env['CPPPATH'] += ['include']
env['CXXFLAGS'] += ['-g', '-Wall']

# Principal: qt moc e fonte
hPrincipal = env.Moc('include/ifg/qt/principal.h')
cPrincipal = env.Object('ifg/qt/principal.cpp')

# MenuPrincipal: qt moc e fonte
hMenuPrincipal = env.Moc('include/ifg/qt/menuprincipal.h')
cMenuPrincipal = env.Object('ifg/qt/menuprincipal.cpp')

# tabuleiro: qt moc e fonte
hTabuleiro = env.Moc('include/ifg/qt/tabuleiro.h')
cTabuleiro = env.Object('ifg/qt/tabuleiro.cpp')

# programa final
env.Program(
	target = 'tabvirt',
	source = [
		'main.cpp',
		# notificacoes
		'ntf/notificacao.cpp',
		# interface QT
		hPrincipal, cPrincipal, hMenuPrincipal, cMenuPrincipal, hTabuleiro, cTabuleiro
	]
)




