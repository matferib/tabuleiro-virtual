  modo_mestre: '',
  modo_visao: 'completo',
  // geral
  nome: '${NOME}',
  raca: '${RACA}',
  template: '',
  tamanho: '${TAMANHO}',  // É possivel ter tamanhos fora do padrao através de magias.
  alinhamento: '${ALINHAMENTO}',
  divindade: '',
  // Cada entrada possui classe e nivel. Ex:
  //classes: [ { classe: 'paladino', nivel: 3 } ],
  classes: [ ${CLASSES} ],
  // String com as chaves de dominio.
  dominios: [ ${DOMINIOS} ],
  // Familiar nao tem base de PV porque depende do PV do personagem.
  familiar: { em_uso: false, chave: '', temporarios: 0, ferimentos: 0, ferimentos_nao_letais: 0 },
  canimal: { raca: '', base: 0, temporarios: 0, ferimentos: 0, ferimentos_nao_letais: 0, notas: '' },
  niveis_negativos: 0,
  // pontos de vida.
  pontos_vida: ${PONTOS_VIDA},
  pontos_vida_temporarios: 0,
  ferimentos: ${FERIMENTOS},  // Valor deve ser >=  0.
  ferimentos_nao_letais: 0,  // Valor deve ser >= 0.
  // experiencia.
  experiencia: ${XP},
  // atributos.
  bonus_atributos: [],
  forca: ${FORCA},
  destreza: ${DESTREZA},
  constituicao: ${CONSTITUICAO},
  inteligencia: ${INTELIGENCIA},
  sabedoria: ${SABEDORIA},
  carisma: ${CARISMA},
  // Cada entrada: { nome, arma_primaria, arma_secundaria}.
  estilos_luta: [],
  // Cada entrada: chave: [ true/false, ... ]
  habilidades_especiais: {},
  // moedas
  platina: ${PECAS_PLATINA},
  ouro: ${PECAS_OURO},
  prata: ${PECAS_PRATA},
  cobre: ${PECAS_COBRE},
  // equipamentos.
  // armas: [ { chave: 'desarmado', nome_gerado: 'desarmado', material: 'nenhum', obra_prima: false, bonus: 0} ],
  armas: [],
  // Cada entrada eh do tipo: { em_uso, chave, material, obra_prima, bonus }
  armaduras: [],
  // Cada entrada { em_uso, chave, material, obra_prima, bonus },
  escudos: [],
  elmo: '',
  // cada entrada: { chave, em_uso }
  aneis: [],
  amuletos: [],
  braceletes: [],
  pocoes: [],
  capas: [],
  chapeus: [],
  outros_equipamentos: '',
  // talentos. Cada chave possui { chave, complemento }, se houver.
  talentos: { gerais: [], guerreiro: [], mago: [], monge: [], ranger: [], outros: [] },

  // pericias: cada entrada possui { chave, pontos, complemento }
  pericias: [],

  // Para magos especialistas. Cada entrada:
  // chave_classe: [],
  escolas_proibidas: {},
  // Feitiços conhecidos, cada entrada:
  // chave_classe: { 0: [ feitico, ... ], 1: [] ...}
  feiticos_conhecidos: { ${FEITICOS_CONHECIDOS} },
  // Slots de feitiços, cada entrada:
  // chave_classe: { 0: [ { nivel, indice, gasto }, ... ], 1: [] ...}
  // Nivel é o nível do feitiço, que pode ser diferente do nível do slot.
  // indice eh o ponteiro para o feitico nos conhecidos para o nível.
  // gasto indica se o feitico está gasto ou não.
  // O último slot é o de dominio (apesar da interface mostrar ele primeiro). Na conversao ele vira uma entrada de dominio especifica.
  slots_feiticos: { ${FEITICOS_LANCAR} },

  notas: '',
