//
//  LightsView.swift
//  IoTROL
//
//  Created by Nikola on 30.05.2023..
//

import SwiftUI

public struct LightsView: View {
    public class ViewModel: ObservableObject {
        @Published var lights: [LightModel]
        
        let confirmAction: ([LightModel]) -> ()
        
        init(lights: [LightModel], confirmAction: @escaping ([LightModel]) -> Void) {
            self.lights = lights
            self.confirmAction = confirmAction
        }
    }
    
    @ObservedObject var viewModel : ViewModel
    
    public init(viewModel: ViewModel) {
        self.viewModel = viewModel
    }
    
    public var body: some View {
        VStack(spacing: 20) {
            Text("Upravljaj svijetlima")
                .font(.system(size: 32, weight: .bold))
            ScrollView(showsIndicators: false) {
                ForEach(Array(viewModel.lights.enumerated()), id: \.element) { index, light in
                    HStack(alignment: .center) {
                        Image(systemName: light.isOn ? "lightbulb" : "lightbulb.slash")
                            .resizable()
                            .scaledToFit()
                            .frame(width: 40, height: 40)
                        Text(light.name)
                            .font(.system(size: 20))
                        Spacer()
                        Toggle("", isOn: $viewModel.lights[index].isOn)
                            .labelsHidden()
                            .tint(.accent)
                    }.padding(.trailing, 5)
                    Divider()
                        .frame(width: UIScreen.main.bounds.width - 40, height: 1)
                        .background(Color.divider)
                }
            }
            Spacer()
            PrimaryButton(label: "Potvrdi") {
                viewModel.confirmAction(viewModel.lights)
            }
        }.padding(.vertical, 40)
            .padding(.horizontal, 40)
            .background(Color.background)
    }
}
