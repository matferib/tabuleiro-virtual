#ifndef NATIVE_H
#define NATIVE_H

#import <Foundation/Foundation.h>
#undef TYPE_BOOL
#include "ifg/tecladomouse.h"
#include "ent/constantes.h"

void nativeCreate(void* view);
void nativeDestroy();
void nativeTimer();
void nativeRender();
void nativeResize(int w, int h);
void nativeTouchPressed(ifg::botoesmouse_e botao, bool toggle, int x, int y);
void nativeTouchMoved(int x, int y);
void nativeTouchReleased();
void nativeDoubleClick(int x, int y);
void nativeScale(float scale);
void nativeRotate(float rad);
void nativeTilt(float delta);
ntf::CentralNotificacoes* nativeCentral();

// Teclado.
void nativeKeyboard(int tecla);
void nativeKeyboardCima();
void nativeKeyboardBaixo();
void nativeKeyboardEsquerda();
void nativeKeyboardDireita();

#endif