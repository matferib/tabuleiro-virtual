#import "GameViewController.h"
#define USAR_GIROSCOPIO 1
#if USAR_GIROSCOPIO
#import <CoreMotion/CoreMotion.h>
#endif
#import <OpenGLES/ES2/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#include "native.h"

#include "ent/entidade.pb.h"
#include "ntf/notificacao.pb.h"


const int TAG_ID = 1;
const int TAG_EVENTOS = 2;
const int TAG_ROTULO = 3;
const int TAG_AURA = 4;
const int TAG_TEXTO_AURA = 5;
const int TAG_TAMANHO = 6;
const int TAG_TEXTO_TAMANHO = 7;
const int TAG_PONTOS_VIDA = 8;
const int TAG_MAX_PONTOS_VIDA = 9;
const int TAG_TIPO_VISAO = 10;
const int TAG_RAIO_VISAO_ESCURO_STEP = 11;
const int TAG_RAIO_VISAO_ESCURO_ROTULO = 12;
const int TAG_RAIO_LUZ_STEPPER = 13;
const int TAG_RAIO_LUZ_ROTULO = 14;
const int TAG_SCROLLVIEW = 15;
const int TAG_VIEW = 16;
const int TAG_BOTAO_OK = 100;
const int TAG_BOTAO_CANCELA = 101;

@interface GameViewController () {
}
@property (strong, nonatomic) EAGLContext* context_;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation GameViewController

- (void)viewDidLoad
{
  [super viewDidLoad];
  GLKView *view = (GLKView *)self.view;

#if USAR_GIROSCOPIO
  // Gyroscope.
  motion_manager_ = [[CMMotionManager alloc] init];
  [motion_manager_ setGyroUpdateInterval:0.1];
#endif
  // Tap.
  UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc]
      initWithTarget:self action:@selector(handleTapGesture:)];
  tapGesture.numberOfTapsRequired = 2;
  [self.view addGestureRecognizer:tapGesture];
  // Pinch.
  UIPinchGestureRecognizer *pinchRecognizer = [[UIPinchGestureRecognizer alloc]
      initWithTarget: self action:@selector(scale:)];
  [self.view addGestureRecognizer:pinchRecognizer];
  [pinchRecognizer setCancelsTouchesInView:NO];
  [pinchRecognizer setDelegate:self];
  // Rotation.
  UIRotationGestureRecognizer *rotationRecognizer =
      [[UIRotationGestureRecognizer alloc]
       initWithTarget:self action:@selector(rotate:)];
  [self.view addGestureRecognizer:rotationRecognizer];
  [rotationRecognizer setCancelsTouchesInView:NO];
  [rotationRecognizer setDelegate:self];

  self.context_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  //self.context_.multiThreaded = TRUE;
  if (!self.context_) {
    NSLog(@"Failed to create ES context");
  }
  NSLog(@"erro: %i", glGetError());

  view.context = self.context_;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
  view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
  view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
  super.preferredFramesPerSecond = ATUALIZACOES_POR_SEGUNDO;

  one_finger_ = true;

  [[UIApplication sharedApplication] setIdleTimerDisabled:YES];

  tipo_visao_array_ = @[@"Normal", @"Visão na Penumbra", @"Visão no Escuro"];

  [self setupGL];
  /*
  CGLError err = 0;
  CGLContextObj ctx = CGLGetCurrentContext();
  
  // Enable the multithreading
  err =  CGLEnable( ctx, kCGLCEMPEngine);
  
  if (err != kCGLNoError )
  {
    // Multithreaded execution may not be available
    // Insert your code to take appropriate action
  }*/
}

- (void)viewDidUnload
{
  [[UIApplication sharedApplication] setIdleTimerDisabled:NO];
  [self tearDownGL];
  [super viewDidUnload];

  if ([EAGLContext currentContext] == self.context_) {
    [EAGLContext setCurrentContext:nil];
  }
}

-(void)viewDidAppear:(BOOL)animated
{
  CGFloat scale = [[UIScreen mainScreen] scale];
  [super viewDidAppear:animated];
  GLKView *view = (GLKView *)self.view;
  nativeResize(view.bounds.size.width * scale,
               view.bounds.size.height * scale);
}

- (void)didReceiveMemoryWarning
{
  [super didReceiveMemoryWarning];

  if ([self isViewLoaded] && ([[self view] window] == nil)) {
    self.view = nil;

    [self tearDownGL];

    if ([EAGLContext currentContext] == self.context_) {
      [EAGLContext setCurrentContext:nil];
    }
    self.context_ = nil;
  }

  // Dispose of any resources that can be recreated.
}

-(GLfloat)viewHeight
{
  GLKView *view = (GLKView *)self.view;
  CGRect bounds = [view bounds];
  return bounds.size.height;
}

-(GLfloat)viewWidth
{
  GLKView *view = (GLKView *)self.view;
  CGRect bounds = [view bounds];
  return bounds.size.width;
}

-(void)scale:(id)sender {
  if ([(UIPinchGestureRecognizer*)sender state] == UIGestureRecognizerStateBegan) {
    last_scale_ = 1.0;
  }

  CGFloat scale = 1.0 - (last_scale_ - [(UIPinchGestureRecognizer*)sender scale]);
  nativeScale(scale);
  last_scale_ = [(UIPinchGestureRecognizer*)sender scale];
}

-(void)rotate:(id)sender {
  if([(UIRotationGestureRecognizer*)sender state] == UIGestureRecognizerStateEnded) {
    last_rotation_ = 0.0;
    return;
  }

  CGFloat rotation =
      0.0 - (last_rotation_ - [(UIRotationGestureRecognizer*)sender rotation]);

  last_rotation_ = [(UIRotationGestureRecognizer*)sender rotation];
  nativeRotate(rotation);
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (void)setupGL
{
  [EAGLContext setCurrentContext:self.context_];
  nativeCreate((__bridge void*)self);
}

- (void)tearDownGL
{
  [EAGLContext setCurrentContext:self.context_];
  nativeDestroy();
}

-(CGPoint)touchAvg:(NSSet*)all_touches
{
  CGPoint ret;
  // media dos toques
  int x = 0, y = 0;
  for (UITouch* touch in all_touches) {
    CGPoint point = [touch locationInView:self.view];
    x += point.x;
    y += point.y;
  }
  ret.x = x / [all_touches count];
  ret.y = y / [all_touches count];

  return ret;
}

-(void)startGyro {
#if USAR_GIROSCOPIO
  UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
  [motion_manager_ startGyroUpdatesToQueue:[NSOperationQueue mainQueue] withHandler:^(CMGyroData *gyro, NSError *error) {
    float delta;
    if (orientation == UIInterfaceOrientationLandscapeLeft) {
      delta = gyro.rotationRate.y;
    } else if (orientation == UIInterfaceOrientationLandscapeRight) {
      delta = -gyro.rotationRate.y;
    } else if (orientation == UIInterfaceOrientationPortrait) {
      delta = gyro.rotationRate.x;
    } else {
      delta = -gyro.rotationRate.x;
    }
    nativeTilt(-delta);
  }];
#endif
}

-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
  CGFloat scale = [[UIScreen mainScreen] scale];
  NSSet* all_touches = [event allTouches];
  GLKView *view = (GLKView *)self.view;
  one_finger_ = [all_touches count] == 1;
  CGPoint point = [self touchAvg:all_touches];
  //NSLog(@"Begin: %0.1f %0.1f touches count: %d", point.x, point.y, [all_touches count]);
  if ([all_touches count] == 2) {
    NSArray* touches_array = [all_touches allObjects];
    CGPoint touch0 = [((UITouch*)touches_array[0]) locationInView:self.view];
    CGPoint touch1 = [((UITouch*)touches_array[1]) locationInView:self.view];
    nativeDoubleTouchPressed(touch0.x * scale, (view.bounds.size.height - touch0.y) * scale,
                             touch1.x * scale, (view.bounds.size.height - touch1.y) * scale);
    [self startGyro];
  }
  nativeTouchPressed(one_finger_ ? ifg::Botao_Esquerdo : ifg::Botao_Direito,
                     false,  // toggle
                     point.x * scale,
                     (view.bounds.size.height - point.y) * scale);
}

-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
  NSSet* all_touches = [event allTouches];
  if (one_finger_ == false && [all_touches count] == 1) {
    // Tava usando dois dedos e um soltou.
    return;
  }
  CGFloat scale = [[UIScreen mainScreen] scale];
  GLKView *view = (GLKView *)self.view;
  CGPoint point = [self touchAvg:all_touches];
  if (one_finger_ && [all_touches count] == 2) {
    //NSLog(@"Switched: %0.1f %0.1f", point.x, point.y);
    one_finger_ = false;
    nativeTouchReleased();
    NSArray* touches_array = [all_touches allObjects];
    CGPoint touch0 = [((UITouch*)touches_array[0]) locationInView:self.view];
    CGPoint touch1 = [((UITouch*)touches_array[1]) locationInView:self.view];
    nativeDoubleTouchPressed(touch0.x * scale, (view.bounds.size.height - touch0.y) * scale,
                             touch1.x * scale, (view.bounds.size.height - touch1.y) * scale);
    [self startGyro];
    nativeTouchPressed(
        ifg::Botao_Direito,
        false,
        point.x * scale,
        (view.bounds.size.height - point.y) * scale);
  } else {
    //NSLog(@"Moved: %0.1f %0.1f", point.x, point.y);
    nativeTouchMoved(point.x * scale,
                     (view.bounds.size.height - point.y) * scale);
  }
  

}

-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
  nativeTouchReleased();
#if USAR_GIROSCOPIO
  [motion_manager_ stopGyroUpdates];
#endif
  one_finger_ = true;
}

- (void)handleTapGesture:(UITapGestureRecognizer *)sender {
  CGFloat scale = [[UIScreen mainScreen] scale];
  if (sender.state == UIGestureRecognizerStateRecognized) {
    GLKView *view = (GLKView *)self.view;
    CGPoint point = [sender locationInView:view];
    nativeDoubleClick(point.x * scale,
                      (view.bounds.size.height - point.y) * scale);
  }
}

-(void)viewWillLayoutSubviews
{
  CGFloat scale = [[UIScreen mainScreen] scale];
  GLKView *view = (GLKView *)self.view;
  CGRect bounds = [view bounds];
  nativeResize(bounds.size.width * scale, bounds.size.height * scale);
}


// Arrendonda valor do slider.
-(void)arredondaAura
{
  int valor = round(slider_aura_.value);
  slider_aura_.value = valor;
  [texto_aura_ setText:[NSString stringWithFormat:@"%.1f m", valor * TAMANHO_LADO_QUADRADO]];
}

// Mudanca no raio de visao.
-(void)mudaRaioVisao
{
  [raio_visao_rotulo_ setText:[NSString stringWithFormat:@"%1.1f m", [raio_visao_stepper_ value]]];
}

// Mudanca no raio de luz.
-(void)mudaRaioLuz
{
  [raio_luz_rotulo_ setText:[NSString stringWithFormat:@"%1.1f m", [raio_luz_stepper_ value]]];
}

-(void)arredondaTamanho
{
  int valor = round(slider_tamanho_.value);
  slider_tamanho_.value = valor;
  [texto_slider_tamanho_ setText:[self tamanhoParaString:valor]];
}

// Tira o prefixo TM, e deixa so primeira maiuscula.
-(NSString*)tamanhoParaString:(int)tamanho
{
  std::string tamanho_cstr = ent::TamanhoEntidade_Name((ent::TamanhoEntidade)tamanho);
  if (tamanho_cstr.size() < 3) {
    NSLog(@"Tamanho: %d", tamanho);
    return @"";
  }
  NSString* tamanho_str = [[NSString stringWithCString:tamanho_cstr.c_str() encoding:NSUTF8StringEncoding] substringFromIndex:3];
  tamanho_str = [tamanho_str capitalizedString];
  //NSLog(@"Retornando Tamanho: %@", tamanho_str);
  return tamanho_str;
}

#pragma mark - Notificacoes
-(bool)trataNotificacao:(const ntf::Notificacao*)notificacao
{
  const ntf::Notificacao& n = *notificacao;
  if (n.tipo() == ntf::TN_ABRIR_DIALOGO_ENTIDADE) {
    UIStoryboard* sb = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    vc_entidade_ = [sb instantiateViewControllerWithIdentifier:@"EntidadeView"];
    notificacao_ = new ntf::Notificacao;
    notificacao_->CopyFrom(n);
    vc_entidade_.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
    //vc.modalPresentationStyle = UIModalPresentationStyle.UIModalPresentationPopover;
    UIView* view = [vc_entidade_ view];

    UITextField* texto_id = (UITextField*)[view viewWithTag:TAG_ID];
    [texto_id setText: [NSString stringWithFormat: @"%d", n.entidade().id()]];

    texto_eventos_ = (UITextView*)[view viewWithTag:TAG_EVENTOS];
    [texto_eventos_.layer setBorderColor:[[[UIColor grayColor]colorWithAlphaComponent:0.5] CGColor]];
    [texto_eventos_.layer setBorderWidth: 1.0];
    texto_eventos_.layer.cornerRadius = 5;
    texto_eventos_.clipsToBounds = YES;
    std::string eventos_str;
    for (const auto& evento : n.entidade().evento()) {
      eventos_str.append(evento.descricao());
      if (evento.has_complemento()) {
        eventos_str.append(" (");
        eventos_str.append(std::to_string(evento.complemento()));
        eventos_str.append(") ");
      }
      eventos_str.append(": ");
      eventos_str.append(std::to_string(evento.rodadas()) + "\n");
    }
    [texto_eventos_ setText: [NSString stringWithCString:eventos_str.c_str()
                                  encoding: NSUTF8StringEncoding]];
    UIButton* botao_ok = (UIButton*)[view viewWithTag:TAG_BOTAO_OK];
    [botao_ok addTarget:self action:@selector(aceitaFechaViewEntidade) forControlEvents:UIControlEventTouchDown];

    UIButton* botao_cancelar = (UIButton*)[view viewWithTag:TAG_BOTAO_CANCELA];
    [botao_cancelar addTarget:self action:@selector(fechaViewEntidade) forControlEvents:UIControlEventTouchDown];

    texto_rotulo_ = (UITextField*)[view viewWithTag:TAG_ROTULO];
    [texto_rotulo_ setText: [NSString stringWithCString:n.entidade().rotulo().c_str() encoding:NSUTF8StringEncoding]];

    tipo_visao_picker_ = (UIPickerView*)[view viewWithTag:TAG_TIPO_VISAO];
    [tipo_visao_picker_ setDelegate:self];
    raio_visao_stepper_ = (UIStepper*)[view viewWithTag:TAG_RAIO_VISAO_ESCURO_STEP];  // criar antes da selecao.
    [raio_visao_stepper_ addTarget:self action:@selector(mudaRaioVisao) forControlEvents:UIControlEventValueChanged];
    raio_visao_rotulo_ = (UILabel*)[view viewWithTag:TAG_RAIO_VISAO_ESCURO_ROTULO];
    [tipo_visao_picker_ selectRow:n.entidade().tipo_visao() inComponent:0 animated:false];
    [self pickerView:tipo_visao_picker_ didSelectRow:(NSInteger)n.entidade().tipo_visao() inComponent:(NSInteger)0];
    [raio_visao_stepper_ setValue:n.entidade().alcance_visao()];
    [self mudaRaioVisao];

    slider_aura_ = (UISlider*)[view viewWithTag:TAG_AURA];
    [slider_aura_ addTarget:self action:@selector(arredondaAura) forControlEvents:UIControlEventValueChanged];
    [slider_aura_ setValue:(n.entidade().aura_m() / TAMANHO_LADO_QUADRADO)];

    texto_aura_ = (UITextField*)[view viewWithTag:TAG_TEXTO_AURA];
    [texto_aura_ setText:[NSString stringWithFormat:@"%.1f m", n.entidade().aura_m()]];

    raio_luz_stepper_ = (UIStepper*)[view viewWithTag:TAG_RAIO_LUZ_STEPPER];
    [raio_luz_stepper_ addTarget:self action:@selector(mudaRaioLuz) forControlEvents:UIControlEventValueChanged];
    float raio_luz = n.entidade().has_luz()
        ? (n.entidade().luz().has_raio_m() ? n.entidade().luz().raio_m() : 4 * TAMANHO_LADO_QUADRADO)
        : 0;
    [raio_luz_stepper_ setValue:raio_luz];

    raio_luz_rotulo_ = (UILabel*)[view viewWithTag:TAG_RAIO_LUZ_ROTULO];
    [self mudaRaioLuz];

    slider_tamanho_ = (UISlider*)[view viewWithTag:TAG_TAMANHO];
    [slider_tamanho_ addTarget:self action:@selector(arredondaTamanho) forControlEvents:UIControlEventValueChanged];
    [slider_tamanho_ setValue:n.entidade().tamanho()];

    texto_slider_tamanho_ = (UITextField*)[view viewWithTag:TAG_TEXTO_TAMANHO];
    [texto_slider_tamanho_ setText:[self tamanhoParaString:n.entidade().tamanho()]];

    pontos_vida_ = (UITextField*)[view viewWithTag:TAG_PONTOS_VIDA];
    [pontos_vida_ setText:[NSString stringWithFormat:@"%d", n.entidade().pontos_vida()]];
    max_pontos_vida_ = (UITextField*)[view viewWithTag:TAG_MAX_PONTOS_VIDA];
    [max_pontos_vida_ setText:[NSString stringWithFormat:@"%d", n.entidade().max_pontos_vida()]];

    CGFloat scale = [[UIScreen mainScreen] scale];
    UIScrollView* scrollview = (UIScrollView*)[view viewWithTag:TAG_SCROLLVIEW];
    UIView* viewproto = (UIView*)[view viewWithTag:TAG_VIEW];
    [scrollview setContentOffset:CGPointMake(0, 0)];
    CGSize tam = viewproto.bounds.size;
    [scrollview setContentSize:tam];
    [scrollview setBackgroundColor:[UIColor whiteColor]];

    [self presentModalViewController:vc_entidade_ animated:TRUE];
    return true;
  }
  return false;
}

-(void)fechaViewEntidade
{
  [vc_entidade_ dismissModalViewControllerAnimated:TRUE];
  vc_entidade_ = nil;
  slider_aura_ = nil;
  raio_luz_stepper_ = nil;
  raio_luz_rotulo_ = nil;
  raio_visao_rotulo_ = nil;
  raio_visao_stepper_ = nil;
  texto_eventos_ = nil;
  texto_rotulo_ = nil;
}

-(void)aceitaFechaViewEntidade
{
  NSArray* subviews = [vc_entidade_.view subviews];
  {
    notificacao_->mutable_entidade()->set_rotulo([
        [[texto_rotulo_ text]
         stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]]
         cStringUsingEncoding:NSUTF8StringEncoding]);
  }
  {
    // Eventos.
    NSString* eventos_str = [texto_eventos_ text];
    notificacao_->mutable_entidade()->clear_evento();
    // Break string.
    for (NSString* str in [eventos_str componentsSeparatedByString:@"\n"]) {
      NSArray* desc_rodadas = [str componentsSeparatedByCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@":()"]];
      if ([[str stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] length] == 0 || [desc_rodadas count] == 0) {
        continue;
      }
      ent::EntidadeProto_Evento evento;
      std::string evento_str(
                             [[[desc_rodadas firstObject]
                               stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]]
                              cStringUsingEncoding:NSUTF8StringEncoding]);
      evento.set_descricao(evento_str);
      for (int i = 1; i < [desc_rodadas count] - 1; ++i) {
        // encontra o elemento nao vazio
        if ([[desc_rodadas[i] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] length] > 0) {
          evento.set_complemento([desc_rodadas[i] intValue]);
        }
      }
      evento.set_rodadas([[desc_rodadas lastObject] intValue]);
      notificacao_->mutable_entidade()->add_evento()->Swap(&evento);
    }
    
  }
  {
    int valor_slider = (int)[slider_aura_ value];
    if (valor_slider > 0) {
      notificacao_->mutable_entidade()->set_aura_m(valor_slider * TAMANHO_LADO_QUADRADO);
    } else {
      notificacao_->mutable_entidade()->clear_aura_m();
    }
  }
  {
    int valor_slider_tamanho = (int)[slider_tamanho_ value];
    notificacao_->mutable_entidade()->set_tamanho((ent::TamanhoEntidade)valor_slider_tamanho);
  }
  {
    int pontos_vida = (int)[[pontos_vida_ text] intValue];
    notificacao_->mutable_entidade()->set_pontos_vida(pontos_vida);
    int max_pontos_vida = (int)[[max_pontos_vida_ text] intValue];
    notificacao_->mutable_entidade()->set_max_pontos_vida(max_pontos_vida);
  }
  {
    notificacao_->mutable_entidade()->set_tipo_visao(static_cast<ent::TipoVisao>([tipo_visao_picker_ selectedRowInComponent:0]));
    float alcance = [raio_visao_stepper_ value];
    notificacao_->mutable_entidade()->set_alcance_visao(
        (alcance == 0.0 && notificacao_->entidade().tipo_visao() == ent::VISAO_ESCURO) ? 18.0f : alcance);
  }
  {
    if ([raio_luz_stepper_ value] == 0.0) {
      notificacao_->mutable_entidade()->clear_luz();
    } else {
      auto* cor_luz = notificacao_->mutable_entidade()->mutable_luz()->mutable_cor();
      notificacao_->mutable_entidade()->mutable_luz()->set_raio_m([raio_luz_stepper_ value]);
      if (!notificacao_->entidade().luz().cor().has_r()) {  // se nao tem r, nao tem gb.
        cor_luz->set_r(1.0f);
        cor_luz->set_g(1.0f);
        cor_luz->set_b(1.0f);
      }
    }
  }
  notificacao_->set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
  //NSLog(@"Notificacao: %s", notificacao_->DebugString().c_str());
  nativeCentral()->AdicionaNotificacao(notificacao_);
  notificacao_ = nullptr;
  [vc_entidade_ dismissModalViewControllerAnimated:TRUE];
  vc_entidade_ = nil;
}

// ---------------------------------------------
// Delegacao de UIPickerView para tipo de visao.
#pragma mark - Delegacao de UIPickerView.
- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component {
  [raio_visao_stepper_ setEnabled:(row == ent::VISAO_ESCURO)];
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component {
  return [tipo_visao_array_ count];
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView {
  return 1;
}

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component {
  return tipo_visao_array_[row];
}

- (UIView *)pickerView:(UIPickerView *)pickerView viewForRow:(NSInteger)row forComponent:(NSInteger)component reusingView:(UIView *)view
{
  UILabel *label = [[UILabel alloc] init];
  label.text = tipo_visao_array_[row];
  label.textAlignment = NSTextAlignmentNatural; //Changed to NS as UI is deprecated.
  return label;
}

// tell the picker the width of each row for a given component
- (CGFloat)pickerView:(UIPickerView *)pickerView widthForComponent:(NSInteger)component {
  int sectionWidth = 300;
  
  return sectionWidth;
}
//----------------------------
// Fim delegacao UIPickerView.
//----------------------------

#pragma mark - Keyboard events
/*-(SEL)seletorLetra: (NSString*)id
{
  SEL s = nil;
  return s;
}*/
-(void)cima
{
  nativeKeyboardCima();;
}
-(void)baixo
{
  nativeKeyboardBaixo();
}
-(void)esquerda
{
  nativeKeyboardEsquerda();
}
-(void)direita
{
  nativeKeyboardDireita();
}

#pragma mark - GLKView and GLKViewController delegate methods

-(NSArray*)keyCommands
{
  return nil;
#if 0
  NSMutableArray* ret = [NSMutableArray alloc];
  /*char c_str[2] = { '\0' };
  for (int i = 0; i < 'z' - 'a'; ++i) {
    c_str[0] = 'a' + i;
    NSString* letra = [NSString stringWithCString: c_str encoding: NSASCIIStringEncoding];
    UIKeyCommand* comando = [UIKeyCommand keyCommandWithInput: letra
                                          modifierFlags: 0
                                          action: [self seletorLetra: letra]];
    [ret addObject: comando];
  }*/
  
  UIKeyCommand* comando_up = [UIKeyCommand keyCommandWithInput:UIKeyInputUpArrow
                                           modifierFlags:0
                                           action:@selector(cima)];
  UIKeyCommand* comando_left = [UIKeyCommand keyCommandWithInput:UIKeyInputLeftArrow
                                             modifierFlags:0
                                             action:@selector(esquerda)];
  UIKeyCommand* comando_right = [UIKeyCommand keyCommandWithInput:UIKeyInputRightArrow
                                              modifierFlags:0
                                              action:@selector(direita)];
  UIKeyCommand* comando_down = [UIKeyCommand keyCommandWithInput:UIKeyInputDownArrow
                                             modifierFlags:0
                                             action:@selector(baixo)];
  [ret addObjectsFromArray: @[
      comando_up,
      comando_down,
      comando_left,
      comando_right]];
  return ret;
#endif
}

-(BOOL)canBecomeFirstResponder
{
  return YES;
}

- (void)update
{
  nativeTimer();
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
  nativeRender();
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer
   shouldRecognizeSimultaneouslyWithGestureRecognizer: (UIGestureRecognizer *)otherGestureRecognizer
{
  return YES;
}

@end

