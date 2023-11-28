//
//  RoomView.swift
//  IoTROL
//
//  Created by Nikola on 29.05.2023..
//

import SwiftUI

public struct RoomView: View {
    
    public class ViewModel: ObservableObject {
        @Published var room: RoomModel?
        @Published var lightsOff = true
        
        let openLights: ([LightModel]) -> ()
        let openBlinds: (BlindsModel) -> ()
        let refreshAction: () -> ()
        
        public init(room: RoomModel? = nil, openLights: @escaping ([LightModel]) -> Void, openBlinds: @escaping (BlindsModel) -> Void, refreshAction: @escaping () -> Void) {
            self.room = room
            self.openLights = openLights
            self.openBlinds = openBlinds
            self.refreshAction = refreshAction
        }
    }
    
    @ObservedObject var viewModel: ViewModel
    
    public init(viewModel: ViewModel) {
        self.viewModel = viewModel
    }
    
    public var body: some View {
        if let room = viewModel.room {
            ScrollView(showsIndicators: false) {
                VStack(spacing: 0) {
                    RoomLineView(title: "Trenutna temperatura: \(room.temp.current)°C", image: room.temp.image)
                        .padding(.bottom, 10)
                    Divider()
                        .frame(width: UIScreen.main.bounds.width - 40, height: 1)
                        .background(Color.divider)
                    RoomLineView(title: "Razina svijetlosti: \(room.luminosity.current) lux", image: room.luminosity.image)
                        .padding(.vertical, 10)
                    Divider()
                        .frame(width: UIScreen.main.bounds.width - 40, height: 1)
                        .background(Color.divider)
                    RoomLineView(title: "Razina vlage: 50 %", image: "humidity")
                        .padding(.vertical, 10)
                    Divider()
                        .frame(width: UIScreen.main.bounds.width - 40, height: 1)
                        .background(Color.divider)
                        .padding(.bottom, 40)
                    
                    RoomCardView(title: "Svijetla", value: viewModel.lightsOff ? "Iskljuceno" : "Upaljeno", image: viewModel.lightsOff ? "lightbulb.slash" : "lightbulb")
                        .onTapGesture {
                            viewModel.openLights(room.lights)
                        }.padding(.bottom, 20)
                    
                    RoomCardView(title: "Rolete", value: "\(Int(room.blinds.current)) %", image: room.blinds.current == 0 ? "blinds.horizontal.closed" : "blinds.horizontal.open")
                        .onTapGesture {
                            viewModel.openBlinds(room.blinds)
                        }
                    
                }.padding(20)
            }.background(Color.background)
                .refreshable {
                    viewModel.refreshAction()
                }
        } else {
            ProgressView()
                .frame(width: UIScreen.main.bounds.width, height: UIScreen.main.bounds.height)
                .background(Color.background)
        }
    }
}

public struct RoomLineView: View {
    var title: String
    var image: String
    
    public var body: some View {
        HStack(spacing: 30) {
            Image(systemName: image).resizable().scaledToFit().frame(width: 32, height: 32)
            Text(title).font(.system(size: 20))
            Spacer()
        }
    }
}

public struct RoomCardView: View {
    var title: String
    var value: String
    var image: String
    
    public var body: some View {
        VStack(spacing: 20) {
            Spacer()
            Text(title).font(.system(size: 16, weight: .bold))
            Image(systemName: image).resizable().scaledToFit().frame(width: 40, height: 40)
            Text(value).font(.system(size: 16, weight: .bold))
            Spacer()
        }.padding(.horizontal, 20)
            .frame(height: 150)
            .frame(maxWidth: .infinity)
            .background(
                RoundedRectangle(cornerRadius: 8)
                    .fill(Color.accent)
            )
    }
}
