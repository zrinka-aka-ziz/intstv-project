//
//  Color+Extension.swift
//  IoTROL
//
//  Created by Nikola on 27.05.2023..
//

import SwiftUI

extension Color {
    public static var accent: Color {
        return Color(UIColor(red: 128/255, green: 173/255, blue: 188/255, alpha: 1.0))
    }
    public static var divider: Color {
        return Color(UIColor(red: 230/255, green: 230/255, blue: 230/255, alpha: 1.0))
    }
    public static var background: Color {
        return Color(UIColor(red: 234/255, green: 232/255, blue: 235/255, alpha: 1.0))
    }
}

extension UIColor {
    public static var accent: UIColor {
        return UIColor(red: 128/255, green: 173/255, blue: 188/255, alpha: 1.0)
    }
    public static var divider: UIColor {
        return UIColor(red: 230/255, green: 230/255, blue: 230/255, alpha: 1.0)
    }
    public static var background: UIColor {
        return UIColor(red: 234/255, green: 232/255, blue: 235/255, alpha: 1.0)
    }
}
