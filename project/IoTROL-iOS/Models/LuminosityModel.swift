//
//  LuminosityModel.swift
//  IoTROL
//
//  Created by Nikola on 29.05.2023..
//

import Foundation

public struct LuminosityModel: Codable {
    public var current: Double
    public var image: String {
        if current > 5 {
            return "light.max"
        } else {
            return "light.min"
        }
    }
}
