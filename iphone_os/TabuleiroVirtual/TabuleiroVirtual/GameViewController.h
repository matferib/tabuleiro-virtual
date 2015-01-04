//
//  GameViewController.h
//  TabuleiroVirtual
//
//  Created by Matheus Ribeiro on 12/21/14.
//  Copyright (c) 2014 Matheus Ribeiro. All rights reserved.
//


#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface GameViewController : GLKViewController <UIGestureRecognizerDelegate>
{
    float last_scale_;
    float last_rotation_;
}

@end
