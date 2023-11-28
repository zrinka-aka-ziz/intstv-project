//
//  TempModel.swift
//  IoTROL
//
//  Created by Nikola on 29.05.2023..
//

import Foundation

public struct TempModel: Codable {
    public var current: Int
    public var image: String {
        if current > 22 {
            return "thermometer.high"
        } else if current < 15 {
            return "thermometer.low"
        } else {
            return "thermometer.medium"
        }
    }
}
