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

@interface GameViewController : GLKViewController <UIGestureRecognizerDelegate, UIPickerViewDelegate>
{
    float last_scale_;
    float last_rotation_;
    bool one_finger_;
    UIViewController* vc_entidade_;  // dialogo de propriedade de entidade.
    ntf::Notificacao* notificacao_;
    UITextField* texto_rotulo_;  // rotulo da entidade.
    UITextView* texto_eventos_;   // Para os eventos da entidade.
    UISlider* slider_aura_;  // para dialogo de propriedade de entidade.
    UITextField* texto_aura_;  // ditto.
    UISlider* slider_tamanho_;  // para dialogo de propriedade de entidade.
    UITextField* texto_slider_tamanho_;  // ditto.
    UITextField* pontos_vida_;
    UITextField* max_pontos_vida_;
    UIPickerView* tipo_visao_picker_;
    UIStepper*  raio_visao_stepper_;
    UILabel* raio_visao_rotulo_;
    UIStepper* raio_luz_stepper_;
    UILabel* raio_luz_rotulo_;
    CMMotionManager* motion_manager_;
    NSArray* tipo_visao_array_;
    
    @public
    NSString* id_cliente_;
    NSString* endereco_servidor_;
}

-(bool)trataNotificacao:(const ntf::Notificacao*)notificacao;

@end
