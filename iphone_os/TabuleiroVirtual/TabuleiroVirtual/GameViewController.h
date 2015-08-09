//
//  GameViewController.h
//  TabuleiroVirtual
//
//  Created by Matheus Ribeiro on 12/21/14.
//  Copyright (c) 2014 Matheus Ribeiro. All rights reserved.
//


#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import <CoreMotion/CoreMotion.h>

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
    UISlider* slider_tamanho_;  // para dialogo de propriedade de entidade.
    UITextField* texto_slider_tamanho_;  // ditto.
    UITextField* pontos_vida_;
    UITextField* max_pontos_vida_;
    CMMotionManager* motion_manager_;
    
    @public
    NSString* id_cliente_;
    NSString* endereco_servidor_;
}

-(bool)trataNotificacao:(const ntf::Notificacao*)notificacao;

@end
