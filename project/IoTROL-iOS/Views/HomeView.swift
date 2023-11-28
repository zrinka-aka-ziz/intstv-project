//
//  HomeView.swift
//  IoTROL
//
//  Created by Nikola on 27.05.2023..
//

import SwiftUI

struct HomeView: View {
    
    public class ViewModel: ObservableObject {
        @Published var rooms: [RoomModel] = []
        
        let openRoom: (RoomModel) -> ()
        
        public init(openRoom: @escaping (RoomModel) -> Void) {
            self.openRoom = openRoom
        }
    }
    
    @ObservedObject var viewModel: ViewModel
    
    public init(viewModel: ViewModel) {
        self.viewModel = viewModel
    }
    
    public var body: some View {
        ScrollView(showsIndicators: false) {
            VStack(spacing: 30) {
                Text("IoTROL iOS app")
                    .font(.system(size: 42, weight: .bold))
                Spacer()
                    .frame(height: 30)
                
                Text("Odaberi sobu: ")
                    .font(.system(size: 32))
                
                VStack(spacing: 10) {
                    LazyVGrid(columns: [GridItem(.flexible()),GridItem(.flexible())], spacing: 10, content: {
                        ForEach(viewModel.rooms, id: \.id) { room in
                            RoomItemView(name: room.name, image: room.image)
                                .onTapGesture {
                                    viewModel.openRoom(room)
                                }
                        }
                    })
                }
                
                Spacer()
            }.padding(20)
        }
        .background(Color.background)
    }
}

struct RoomItemView: View {
    var name: String
    var image: String
    
    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: image)
                .resizable()
                .scaledToFit()
                .frame(width: 32, height: 32)
                .padding(.top, 24)
            Text(name)
                .font(.system(size: 16, weight: .bold))
            Spacer()
        }.padding(.horizontal, 20)
            .frame(height: 120)
            .frame(maxWidth: .infinity)
            .background(
                RoundedRectangle(cornerRadius: 8)
                    .fill(Color.accent)
            )
    }
}
