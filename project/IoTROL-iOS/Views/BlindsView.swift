//
//  BlindsView.swift
//  IoTROL
//
//  Created by Nikola on 30.05.2023..
//

import SwiftUI

public struct BlindsView: View {
    public class ViewModel: ObservableObject {
        @Published var blinds: BlindsModel
        
        let confirmAction: (Double) -> ()
        
        public init(blinds: BlindsModel, confirmAction: @escaping (Double) -> ()) {
            self.blinds = blinds
            self.confirmAction = confirmAction
        }
    }
    
    @ObservedObject var viewModel: ViewModel
    
    public init(viewModel: ViewModel) {
        self.viewModel = viewModel
    }
    
    public var body: some View {
        VStack(spacing: 20) {
            Text("Upravljaj roletama")
                .font(.system(size: 32, weight: .bold))
            Image(systemName: viewModel.blinds.current == 0 ? "blinds.horizontal.closed" : "blinds.horizontal.open")
                .resizable()
                .scaledToFit()
                .frame(width: 40, height: 40)
            Text("\(Int(viewModel.blinds.current)) %")
                .font(.system(size: 20))
            Slider(value: $viewModel.blinds.current, in: 0.0...100.0, step: 1.0)
                .tint(.accent)
            Spacer()
            PrimaryButton(label: "Potvrdi") {
                viewModel.confirmAction(viewModel.blinds.current)
            }
        }.frame(maxWidth: .infinity)
            .padding(.vertical, 40)
            .padding(.horizontal, 40)
            .background(Color.background)
    }
}
