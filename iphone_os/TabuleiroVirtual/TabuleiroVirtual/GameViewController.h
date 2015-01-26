//
//  GameViewController.h
//  TabuleiroVirtual
//
//  Created by Matheus Ribeiro on 12/21/14.
//  Copyright (c) 2014 Matheus Ribeiro. All rights reserved.
//


#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

namespace ntf {
class Notificacao;
}  // namespace ntf

@interface GameViewController : GLKViewController <UIGestureRecognizerDelegate>
{
    float last_scale_;
    float last_rotation_;
    bool one_finger_;
    UIViewController* vc_entidade_;  // dialogo de propriedade de entidade.
    ntf::Notificacao* notificacao_;
    UISlider* slider_;  // para dialogo de propriedade de entidade.
    UITextField* texto_slider_;  // ditto.
}

-(bool)trataNotificacao:(const ntf::Notificacao*)notificacao;

@end
