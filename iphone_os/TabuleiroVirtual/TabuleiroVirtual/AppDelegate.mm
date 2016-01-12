#import "AppDelegate.h"
#import "GameViewController.h"

#define VIEW_ID 1
#define VIEW_IP 2
#define VIEW_CONECTAR 100
#define VIEW_SERVIDOR 101

@interface AppDelegate ()

@end

@implementation AppDelegate


-(BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    return YES;
}

-(void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

-(void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

-(void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

-(void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    UITextField* texto_id = (UITextField*)[self.window viewWithTag:VIEW_ID];
    NSString* host_antes_ponto = [[NSProcessInfo processInfo] hostName];
    host_antes_ponto = [[host_antes_ponto componentsSeparatedByString:@"."] firstObject];
    [texto_id setText:host_antes_ponto];
    UIButton* botao_servidor = (UIButton*)[self.window viewWithTag:VIEW_SERVIDOR];
    [botao_servidor addTarget:self action:@selector(servidor)
             forControlEvents:UIControlEventTouchDown];
    UIButton* botao_conectar = (UIButton*)[self.window viewWithTag:VIEW_CONECTAR];
    [botao_conectar addTarget:self action:@selector(conectar)
             forControlEvents:UIControlEventTouchDown];
}

-(void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

-(void)conectar {
    UIStoryboard* sb = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    GameViewController* game_view_controller = (GameViewController*)
        [sb instantiateViewControllerWithIdentifier:@"GameViewController"];
    game_view_controller->id_cliente_ =
        [(UITextField*)[self.window viewWithTag:VIEW_ID] text];
    game_view_controller->endereco_servidor_ =
        [(UITextField*)[self.window viewWithTag:VIEW_IP] text];
    UIViewController* responder = (UIViewController*)
        [[[self.window subviews] firstObject] nextResponder];
    [responder presentModalViewController:game_view_controller animated:TRUE];
}

-(void)servidor {
    UIStoryboard* sb = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    GameViewController* game_view_controller = (GameViewController*)
        [sb instantiateViewControllerWithIdentifier:@"GameViewController"];
    UIViewController* responder = (UIViewController*)
        [[[self.window subviews] firstObject] nextResponder];
    [responder presentModalViewController:game_view_controller animated:TRUE];
}

@end
