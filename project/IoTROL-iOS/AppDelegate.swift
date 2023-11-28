//
//  AppDelegate.swift
//  IoTROL
//
//  Created by Nikola on 27.05.2023..
//

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {
    
    var window: UIWindow?


    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        // Override point for customization after application launch.
        
        window = UIWindow(frame: UIScreen.main.bounds)
        window?.overrideUserInterfaceStyle = .light
        let nav = UINavigationController(rootViewController: MainViewController())
        nav.navigationBar.tintColor = .black
        window?.rootViewController = nav
        window?.makeKeyAndVisible()
        
        return true
    }
}
