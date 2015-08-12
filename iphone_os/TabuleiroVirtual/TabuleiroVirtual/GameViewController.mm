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

  self.context_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
  if (!self.context_) {
    NSLog(@"Failed to create ES context");
  }

  view.context = self.context_;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
  view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
  view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
  super.preferredFramesPerSecond = ATUALIZACOES_POR_SEGUNDO;
  
  one_finger_ = true;

  [self setupGL];
}

- (void)viewDidUnload
{
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

-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
  CGFloat scale = [[UIScreen mainScreen] scale];
  NSSet* all_touches = [event allTouches];
  GLKView *view = (GLKView *)self.view;
  one_finger_ = [all_touches count] == 1;
  CGPoint point = [self touchAvg:all_touches];
  //NSLog(@"Begin: %0.1f %0.1f", point.x, point.y);
  nativeTouchPressed(
      one_finger_ ? ifg::Botao_Esquerdo : ifg::Botao_Direito,
      false,  // toggle
      point.x * scale,
      (view.bounds.size.height - point.y) * scale);
#if USAR_GIROSCOPIO
  UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
  if ([all_touches count] == 2) {
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
      nativeTilt(delta);
    }];
  }
#endif
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
-(void)arredonda
{
  int valor = round(slider_.value);
  slider_.value = valor;
  [texto_slider_ setText:[NSString stringWithFormat:@"%d", valor]];
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
  if (n.tipo() == ntf::TN_ERRO || n.tipo() == ntf::TN_INFO) {
    // Deprecated ios8 mas funciona em ipad1.
    NSString* msg_str = [NSString stringWithCString:n.erro().c_str() encoding:NSUTF8StringEncoding];
    NSString* titulo_str = n.tipo() == ntf::TN_INFO ? @"Info" : @"Erro";
    UIAlertView* alert = [[UIAlertView alloc] initWithTitle:titulo_str
                                              message:msg_str
                                              delegate:nil
                                              cancelButtonTitle:@"OK"
                                              otherButtonTitles:nil];
    [alert show];
    // Nao funciona ipad1.
    /*UIAlertController *alert = nil;
    UIAlertAction* defaultAction = nil;
    alert = [UIAlertController alertControllerWithTitle:titulo_str
                               message:msg_str
                               preferredStyle:UIAlertControllerStyleAlert];
    defaultAction = [UIAlertAction actionWithTitle: @"OK"
                                             style: UIAlertActionStyleDefault
                                           handler: ^(UIAlertAction * action) {}];
    [alert addAction:defaultAction];
    [self presentViewController:alert animated:YES completion:nil];*/
    return true;
  } else if (n.tipo() == ntf::TN_ABRIR_DIALOGO_ENTIDADE) {
    UIStoryboard* sb = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    vc_entidade_ = [sb instantiateViewControllerWithIdentifier:@"EntidadeView"];
    notificacao_ = new ntf::Notificacao;
    notificacao_->CopyFrom(n);
    vc_entidade_.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
    //vc.modalPresentationStyle = UIModalPresentationStyle.UIModalPresentationPopover;
    UIView* view = [vc_entidade_ view];

    UITextField* texto_id = (UITextField*)[view viewWithTag:TAG_ID];
    [texto_id setText: [NSString stringWithFormat: @"%d", n.entidade().id()]];

    UITextView* text_view = (UITextView*)[view viewWithTag:TAG_EVENTOS];
    [text_view.layer setBorderColor:[[[UIColor grayColor]colorWithAlphaComponent:0.5] CGColor]];
    [text_view.layer setBorderWidth: 1.0];
    text_view.layer.cornerRadius = 5;
    text_view.clipsToBounds = YES;
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
    [text_view setText: [NSString stringWithCString:eventos_str.c_str()
                                           encoding: NSUTF8StringEncoding]];
    UIButton* botao_ok = (UIButton*)[view viewWithTag:TAG_BOTAO_OK];
    [botao_ok addTarget:self action:@selector(aceitaFechaViewEntidade) forControlEvents:UIControlEventTouchDown];

    UIButton* botao_cancelar = (UIButton*)[view viewWithTag:TAG_BOTAO_CANCELA];
    [botao_cancelar addTarget:self action:@selector(fechaViewEntidade) forControlEvents:UIControlEventTouchDown];

    UITextField* texto_rotulo = (UITextField*)[view viewWithTag:TAG_ROTULO];
    [texto_rotulo setText: [NSString stringWithCString:n.entidade().rotulo().c_str() encoding:NSUTF8StringEncoding]];

    slider_ = (UISlider*)[view viewWithTag:TAG_AURA];
    [slider_ addTarget:self action:@selector(arredonda) forControlEvents:UIControlEventValueChanged];
    [slider_ setValue:n.entidade().aura()];

    texto_slider_ = (UITextField*)[view viewWithTag:TAG_TEXTO_AURA];
    [texto_slider_ setText:[NSString stringWithFormat:@"%d", n.entidade().aura()]];

    slider_tamanho_ = (UISlider*)[view viewWithTag:TAG_TAMANHO];
    [slider_tamanho_ addTarget:self action:@selector(arredondaTamanho) forControlEvents:UIControlEventValueChanged];
    [slider_tamanho_ setValue:n.entidade().tamanho()];

    texto_slider_tamanho_ = (UITextField*)[view viewWithTag:TAG_TEXTO_TAMANHO];
    [texto_slider_tamanho_ setText:[self tamanhoParaString:n.entidade().tamanho()]];

    pontos_vida_ = (UITextField*)[view viewWithTag:TAG_PONTOS_VIDA];
    [pontos_vida_ setText:[NSString stringWithFormat:@"%d", n.entidade().pontos_vida()]];
    max_pontos_vida_ = (UITextField*)[view viewWithTag:TAG_MAX_PONTOS_VIDA];
    [max_pontos_vida_ setText:[NSString stringWithFormat:@"%d", n.entidade().max_pontos_vida()]];

    [self presentModalViewController:vc_entidade_ animated:TRUE];
    return true;
  }
  return false;
}

-(void)fechaViewEntidade
{
  [vc_entidade_ dismissModalViewControllerAnimated:TRUE];
  vc_entidade_ = nil;
  slider_ = nil;
}

-(void)aceitaFechaViewEntidade
{
  NSArray* subviews = [vc_entidade_.view subviews];
  for (UIView* subview in subviews) {
    switch ([subview tag]) {
      case TAG_EVENTOS: {
        // Eventos.
        UITextView* text_view = (UITextView*)subview;
        NSString* eventos_str = [text_view text];
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

        break;
      }
      case TAG_ROTULO: {
        UITextField* texto_rotulo = (UITextField*)subview;
        notificacao_->mutable_entidade()->set_rotulo(
            [[[texto_rotulo text]
              stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]]
                  cStringUsingEncoding:NSUTF8StringEncoding]);
        break;
      }
    }
  }
  {
    int valor_slider = (int)[slider_ value];
    if (valor_slider > 0) {
      notificacao_->mutable_entidade()->set_aura(valor_slider);
    } else {
      notificacao_->mutable_entidade()->clear_aura();
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
  notificacao_->set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
  nativeCentral()->AdicionaNotificacao(notificacao_);
  notificacao_ = nullptr;
  [vc_entidade_ dismissModalViewControllerAnimated:TRUE];
  vc_entidade_ = nil;
  slider_ = nil;
}

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

