//
//  LightModel.swift
//  IoTROL
//
//  Created by Nikola on 29.05.2023..
//

import Foundation

public struct LightModel: Hashable, Codable {
    public var id = UUID()
    public var name: String
    public var isOn: Bool
}
