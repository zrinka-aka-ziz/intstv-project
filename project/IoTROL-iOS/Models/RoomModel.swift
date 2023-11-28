//
//  RoomModel.swift
//  IoTROL
//
//  Created by Nikola on 27.05.2023..
//

import Foundation

public struct RoomModel: Codable {
    public var id = UUID()
    public var name: String
    public var type: RoomEnum
    public var image: String {
        return type.rawValue
    }
    public var temp: TempModel
    public var luminosity: LuminosityModel
    public var lights: [LightModel]
    public var blinds: BlindsModel
}

public enum RoomEnum: String, Codable {
    case livingRoom = "sofa"
    case bedRoom = "bed.double"
    case bathRoom = "toilet"
    case kitchen = "refrigerator"
    case lounge = "fireplace"
    case other = "door.french.open"
}
