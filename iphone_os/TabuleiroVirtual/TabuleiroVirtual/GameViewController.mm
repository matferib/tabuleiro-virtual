//
//  GameViewController.m
//  TabuleiroVirtual
//
//  Created by Matheus Ribeiro on 12/21/14.
//  Copyright (c) 2014 Matheus Ribeiro. All rights reserved.
//

#import "GameViewController.h"
#import <OpenGLES/ES2/glext.h>
#import <UIKit/UIKit.h>
#include "native.h"

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
  super.preferredFramesPerSecond = 30;
  
  one_finger_ = true;

  [self setupGL];
}

-(void)viewDidAppear:(BOOL)animated
{
  CGFloat scale = [[UIScreen mainScreen] scale];
  [super viewDidAppear:animated];
  GLKView *view = (GLKView *)self.view;
  nativeResize(view.bounds.size.width * scale,
               view.bounds.size.height * scale);
}

- (void)viewDidUnload
{
  [self tearDownGL];
  [super viewDidUnload];
    
  if ([EAGLContext currentContext] == self.context_) {
    [EAGLContext setCurrentContext:nil];
  }
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
  nativeCreate();
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
  nativeTouchPressed(
      one_finger_ ? ifg::Botao_Esquerdo : ifg::Botao_Direito,
      false,  // toggle
      point.x * scale,
      (view.bounds.size.height - point.y) * scale);
}

-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
  CGFloat scale = [[UIScreen mainScreen] scale];
  NSSet* all_touches = [event allTouches];
  GLKView *view = (GLKView *)self.view;
  CGPoint point = [self touchAvg:all_touches];
  if (one_finger_ && [all_touches count] == 2) {
    one_finger_ = false;
    nativeTouchReleased();
    nativeTouchPressed(
        ifg::Botao_Direito,
        false,
        point.x * scale,
        (view.bounds.size.height - point.y) * scale);
  } else {
    nativeTouchMoved(point.x * scale,
                     (view.bounds.size.height - point.y) * scale);
  }
}

-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
  nativeTouchReleased();
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

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
  nativeTimer();
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
  nativeRender();
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer
   shouldRecognizeSimultaneouslyWithGestureRecognizer:
       (UIGestureRecognizer *)otherGestureRecognizer
{
  return YES;
}

@end

