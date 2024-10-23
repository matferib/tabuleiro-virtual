import os
import sys

env = Environment(ENV=os.environ, toolpath=['tools'], tools=['protoc'])

sistema = ARGUMENTS.get('sistema', 'linux')
if sistema not in ['win32', 'linux', 'apple']:
  sys.exit('Sistema invalido: ' + sistema)
print('Usando sistema: ' + sistema)

if sistema == 'win32':
  env.Tool('mingw')
else:
  env.Tool('default')
  #env['CXX'] = 'clang++-10'

# qt
env.SConscript('localqt.SConscript', exports = 'env')
env['QT_MOCHSUFFIX'] = '.cpp'
if sistema == 'win32':
  if 'QTDIR' not in env:
	  env['QTDIR'] = 'd:/Qt/'
  #env['QT_LIBPATH'] = 'win32/lib'
  env['QT_LIBPATH'] = 'd:/Qt/lib'
  env['QT_LIB'] = ['QtOpenGL', 'QtGui', 'QtCore', 'QtMultimedia']
elif sistema == 'apple':
  if 'QTDIR' not in env:
    env['QTDIR'] = '/opt/homebrew/opt/qt@5/'
  if 'QT5DIR' not in env:
    env['QT5DIR'] = '/opt/homebrew/opt/qt@5/'
  env['QT5FRAMEWORKPATH'] = env['QT5DIR'] + 'Frameworks/'
  env['FRAMEWORKPATH'] = [env['QT5FRAMEWORKPATH']]
  frameworks = ['QtOpenGL', 'QtGui', 'QtWidgets', 'QtCore', 'QtMultimedia', 'OpenGL']
  env['FRAMEWORKS'] = frameworks
  #env['QT5_CPPPATH'] = [env['QT5FRAMEWORKPATH'] + '/QtWidgets.framework/Headers', ]
  env['QT5_CPPPATH'] = list(map(lambda x: env['QT5FRAMEWORKPATH'] + x + '.framework/Headers', frameworks))
  print(env['QT5_CPPPATH'])
  #env['QT_CPPPATH'] = [env['QTDIR'] + '/include/QtGui', env['QTDIR'] + '/include/QtCore', env['QTDIR'] + '/include/QtMultimedi    a', env['QTDIR'] + '/include/', env['QTDIR'] + '/include/QtOpenGL', env['QTDIR'] + '/include/QtWidgets']
  env['QT_LIB'] = []
  env['RPATH'] = []
else:
  if 'QTDIR' not in env:
    env['QTDIR'] = '../libs/Qt/5.15.0/gcc_64/'
  env['QT5_CPPPATH'] = [env['QTDIR'] + '/include/QtGui', env['QTDIR'] + '/include/QtCore', env['QTDIR'] + '/include/QtMultimedia', env['QTDIR'] + '/include/', env['QTDIR'] + '/include/QtOpenGL', env['QTDIR'] + '/include/QtWidgets']
  env['QT_LIBPATH'] = env['QTDIR'] + '/lib'
  env['QT_LIB'] = ['Qt5Gui', 'Qt5OpenGL', 'Qt5Core', 'Qt5Widgets', 'Qt5Multimedia']
  env['RPATH'] = [env['QT_LIBPATH']]
qt = Tool('qt5')
qt(env)

# protobuffer.
env['PROTOCOUTDIR'] = './'
env['PROTOCPYTHONOUTDIR'] = None

# c++
debug = ARGUMENTS.get('debug', '1')
print('Usando debug: %r' % debug)
if sistema == 'win32':
  env['CPPPATH'] += ['./', 'win32/include']
  env['CPPDEFINES'] = {'USAR_GLOG': 0, 'USAR_GFLAGS': 0, 'WIN32_LEAN_AND_MEAN': 1, 'WIN32': 1, '_WINDOWS': 1, '_CRT_SECURE_NO_WARNINGS': 1, '_WIN32_WINNT': 0x0601, 'WINVER': 0x0601, 'QT_STATIC_BUILD': 1 }
  env['CXXFLAGS'] = ['-std=c++11', '-Wall', '-Wfatal-errors']
  env['LIBS'] += ['glu32', 'opengl32', 'protobuf', 'boost_filesystem-mgw48-mt-1_55', 'boost_system-mgw48-mt-1_55', 'boost_timer-mgw48-mt-1_55', 'boost_chrono-mgw48-mt-1_55', 'boost_date_time-mgw48-mt-1_55', 'ws2_32', 'Mswsock', 'Gdi32', 'Winmm', 'ole32', 'Oleacc', 'OleAut32', 'libuuid', 'Comdlg32', 'imm32', 'Winspool']
  env['LIBPATH'] += [ 'win32/lib' ]
  env['LINKFLAGS'] = ['-Wl,--subsystem,windows']
elif sistema == 'apple':
  env['CPPPATH'] = ['./', '/usr/local/include'] + env['QT5_CPPPATH'] + ['/opt/homebrew/include']
  env['CPPDEFINES'] = {'USAR_GLOG': 1, 'USAR_GFLAGS': 0, "_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR": 1 }
  #env['CPPDEFINES'] = {'USAR_GLOG': 0, 'USAR_GFLAGS': 0}
  env['CXXFLAGS'] += ['-Wall', '-std=c++17', '-stdlib=libc++', '-Wno-deprecated-register', '-Wno-deprecated-declarations', '-Wno-unused-local-typedef', '-Wfatal-errors', '-Wno-unused-lambda-capture']
  #env['CXXFLAGS'] += ['-Wall', '-std=c++17', '-stdlib=libstdc++', '-Wno-deprecated-register', '-Wno-deprecated-declarations', '-Wno-unused-local-typedef', '-Wfatal-errors', '-Wno-unused-lambda-capture']
  env['LIBPATH'] = [ '/usr/local/lib', '/opt/homebrew/lib' ]
  env['LIBS'] = ['protobuf', 'boost_system', 'boost_timer', 'boost_filesystem', 'boost_date_time', 'pthread', 'absl_strings', 'absl_log_entry', 'absl_log_sink', 'absl_log_initialize', 'absl_log_globals', 'absl_log_severity', 'absl_log_internal_log_sink_set', 'absl_log_internal_message', 'absl_log_internal_format', 'absl_log_internal_globals', 'absl_log_internal_fnmatch', 'absl_log_internal_nullguard', 'absl_log_entry', 'absl_log_flags', 'absl_log_globals', 'absl_log_initialize', 'absl_log_internal_check_op', 'absl_log_internal_conditions', 'absl_log_internal_fnmatch', 'absl_log_internal_format', 'absl_log_internal_globals', 'absl_log_internal_log_sink_set', 'absl_log_internal_message', 'absl_log_internal_nullguard', 'absl_log_internal_proto', 'absl_log_severity', 'absl_log_sink', 'absl_vlog_config_internal', 'absl_hash', 'absl_str_format_internal', 'absl_log_initialize', 'absl_flags_parse']
  env['LINKFLAGS'] = ['-headerpad_max_install_names']
else:
  # linux.
  env['CPPPATH'] += ['./', '../libs/tbb/include'] + env['QT_CPPPATH']
  env['CPPDEFINES'] = {'USAR_GLOG': 0, 'USAR_GFLAGS': 0, 'USAR_WATCHDOG': 1}
  env['CXXFLAGS'] = ['-Wall', '-std=c++17', '-Wfatal-errors', '-fPIC']
  env['LIBPATH'] += [ '../libs/tbb/build/linux_intel64_gcc_cc9.3.0_libc2.31_kernel5.4.0_release/' ]
  env['LIBS'] += ['GLU', 'GL', 'protobuf', 'boost_timer', 'boost_chrono', 'boost_filesystem', 'boost_system', 'boost_date_time', 'pthread', 'tbb']
  #env['LINKFLAGS'] += ['-stdlib=libc++']

# Configuracoes locais.
env.SConscript('local.SConscript', exports = 'env')


if (debug == '1'):
  env['CXXFLAGS'] += ['-g']
  env['CPPDEFINES']['DEBUG'] = '1'
else:
  env['CXXFLAGS'] += ['-O2']
  env['CPPDEFINES']['RELEASE'] = '1'

#print env.Dump()

usar_opengl_es = (ARGUMENTS.get('usar_opengl_es', '0') == '1')
print('usar_opengl_es : %r' % usar_opengl_es)
if usar_opengl_es:
  env['CPPPATH'] += ['./opengl_es/']
  env['CPPDEFINES']['USAR_OPENGL_ES'] = 1
  if sistema != 'apple':
    env['LIBS'] += ['GLESv1_CM']

gerar_profile = (ARGUMENTS.get('gerar_profile', '0') == '1')
if gerar_profile:
  env['CXXFLAGS'] += ['-pg']
  env['LINKFLAGS'] += ['-pg']

usar_zbuffer_16_bits = (ARGUMENTS.get('usar_zbuffer_16_bits', '0') == '1')
if usar_zbuffer_16_bits:
  env['CPPDEFINES']['ZBUFFER_16_BITS'] = 1
else:
  env['CPPDEFINES']['ZBUFFER_16_BITS'] = 0

env['CPPDEFINES']['GL_GLEXT_PROTOTYPES'] = 1
env['CPPDEFINES']['USAR_QT'] = 1
env['CPPDEFINES']['USAR_QT5'] = 1

# Daqui pra baixo eh tudo comum.

# Teclado.
cTecladoMouse = env.Object('ifg/tecladomouse.cpp')
cInterface = env.Object('ifg/interface.cpp')
# Permite lambdas no QT.
cUtil = env.Object('ifg/qt/util.cpp')
ifg_proto = env.Protoc(
  target = [],
  source = ['ifg/modelos.proto'],
)

# Principal: qt moc e fonte. Os mocs sao gerados automaticamente
# se estiverem no mesmo diretorio do fonte.
cPrincipal = env.Object('ifg/qt/principal.cpp')

# MenuPrincipal: qt moc e fonte
cMenuPrincipal = env.Object('ifg/qt/menuprincipal.cpp')

# visualizador3d: qt moc e fonte
cVisualizador3d = env.Object('ifg/qt/visualizador3d.cpp')
cVisualizador3dDialogos = env.Object('ifg/qt/visualizador3d_dialogos.cpp')

# Atualizacao de UI de entidade.
cAtualizaUI = env.Object('ifg/qt/atualiza_ui.cpp')

cQtInterface = env.Object('ifg/qt/qt_interface.cpp')

cQtPericiasUtil = env.Moc5('ifg/qt/pericias_util.h')
cQtEventoUtil = env.Moc5('ifg/qt/evento_util.h')
cQtTalentosUtil = env.Moc5('ifg/qt/talentos_util.h')

# Implementacao das texturas.
#cTexturas = env.Object('ifg/qt/texturas.cpp')
cTexturasLode = env.Object('tex/lodepng.cpp')
cTexturas = env.Object('tex/texturas.cpp')

# Modelos 3d.
cModelos3d = env.Object('m3d/m3d.cpp')

# ent
cTabuleiro = env.Object('ent/tabuleiro.cpp')
cTabuleiroControleVirtual = env.Object('ent/tabuleiro_controle_virtual.cpp')
cTabuleiroTratador = env.Object('ent/tabuleiro_tratador.cpp')
cTabuleiroInterface = env.Object('ent/tabuleiro_interface.cpp')
cTabuleiroPicking = env.Object('ent/tabuleiro_picking.cpp')
cEntidade = env.Object('ent/entidade.cpp')
cTabelas = env.Object('ent/tabelas.cpp')
cEntidadeComposta = env.Object('ent/entidade_composta.cpp')
cEntidadeForma = env.Object('ent/entidade_forma.cpp')
cAcoes = env.Object('ent/acoes.cpp')
cConstantes = env.Object('ent/constantes.cpp')
cEntUtil = env.Object('ent/util.cpp')
cEntRecomputa = env.Object('ent/recomputa.cpp')
cEntDesenho = env.Object('ent/entidade_desenho.cpp')
if sistema == 'linux':
  cEntWatchdog = env.Object('ent/watchdog.cpp')
ent_proto = env.Protoc(
  target = [],
  source = ['ent/entidade.proto', 'ent/tabuleiro.proto', 'ent/acoes.proto', 'ent/controle_virtual.proto', 'ent/comum.proto', 'ent/tabelas.proto'],
)

# arq
cArq = env.Object('arq/arquivo.cpp')
cArqPc = env.Object('arq/arquivo_pc.cpp')

# net
cNetServidor = env.Object('net/servidor.cpp')
cNetCliente = env.Object('net/cliente.cpp')
cNetUtil = env.Object('net/util.cpp')
cNetSocket = env.Object('net/socket.cpp')

# som.
cSom = env.Object('som/som_qt.cpp')

# Notificacoes.
cNtf = env.Object('ntf/notificacao.cpp')
ntf_proto = env.Protoc(
  target = [],
  source = ['ntf/notificacao.proto'],
)

# GL
cGl = env.Object('gltab/gl_es.cpp') if usar_opengl_es else env.Object('gltab/gl_normal.cpp')
cGlComum = env.Object('gltab/gl_comum.cpp')
cGlChar = env.Object('gltab/gl_char.cpp')
cGlVbo = env.Object('gltab/gl_vbo.cpp')
cGlues = env.Object('gltab/glues.cpp')

# Matrix
cMatrix = env.Object('matrix/matrices.cpp')

objetos = [
    # som.
    cSom,
    # net.
    cNetServidor, cNetCliente, cNetUtil, cNetSocket,
    # notificacoes.
    ntf_proto[0], cNtf,
    # interface.
    cTecladoMouse, cInterface, ifg_proto[0],
    # interface QT
    cPrincipal, cMenuPrincipal, cVisualizador3d, cVisualizador3dDialogos, cUtil, cQtInterface, cQtPericiasUtil, cQtEventoUtil, cQtTalentosUtil, cAtualizaUI,
    # Texturas
    cTexturas, cTexturasLode,
    # Modelos3d.
    cModelos3d,
    # ent. Os protos sao de 2 em 2 para nao incluir os cabecalhos.
    ent_proto[0], ent_proto[2], ent_proto[4], ent_proto[6], ent_proto[8], ent_proto[10],
    cTabuleiro, cTabuleiroControleVirtual, cTabuleiroPicking, cTabuleiroInterface, cTabuleiroTratador,
    cTabelas, cEntidade, cEntidadeComposta, cEntidadeForma, cAcoes, cConstantes, cEntUtil, cEntRecomputa, cEntDesenho,
    # gl.
    cGlComum, cGl, cGlChar, cGlVbo, cGlues,
    # arq
    cArq, cArqPc,
    # matrix
    cMatrix,
] + ([ cEntWatchdog ] if sistema == 'linux' else [])


# programa final
env.Default(env.Program(
  target = 'tabvirt',
  source = [ 'main.cpp', ] + objetos
))

compilar_testes = (ARGUMENTS.get('testes', '0') == '1')
print('compilar_testes : %r' % compilar_testes)
if compilar_testes:
  env['CPPPATH'] += ['./googletest/googletest/googletest/include', './googletest/googletest/googlemock/include']
  if sistema == 'apple':
    env['LIBPATH'] += ['./googletest/googletest/bazel-bin']
    env['RPATH'] += ['./googletest/googletest/bazel-bin']
  else:
    env['LIBPATH'] += ['./googletest/googletest/lib/', './googletest/googletest/bazel-bin']
  env['LIBS'] += ['pthread', 'gtest']
  #env['RPATH'] = ['./googletest/googletest/lib/', './googletest/googletest/bazel-bin']
  env.Program(
      target = 'teste_ent_ent',
      source = ['ent/ent_test.cpp' ] + objetos)
  env.Program(
      target = 'teste_ent_acoes',
      source = ['ent/acoes_test.cpp' ] + objetos)
  env.Program(
      target = 'teste_ent_util',
      source = ['ent/util_test.cpp' ] + objetos)
  env.Program(
      target = 'teste_modelos',
      source = ['ifg/modelos_test.cpp' ] + objetos)
  env.Program(
      target = 'teste_arquivo',
      source = ['arq/arquivo_test.cpp' ] + objetos)
  env.Program(
      target = 'teste_net_util',
      source = ['net/util_test.cpp' ] + objetos)

rodar_benchmark = (ARGUMENTS.get('benchmark', '0') == '1')
print('benchmark : %r' % rodar_benchmark)
if rodar_benchmark:
  env['CPPDEFINES']['BENCHMARK'] = '1'
  env['CPPDEFINES']['USAR_WATCHDOG'] = '0'
  if sistema == 'apple':
    env['FRAMEWORKS'] += ['GLUT']
  else:
    env['LIBS'] += ['glut']
  env.Program(
      target = 'benchmark',
      source = ['benchmark.cpp', ] + objetos)
