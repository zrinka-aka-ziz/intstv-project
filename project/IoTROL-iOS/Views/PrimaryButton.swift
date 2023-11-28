//
//  PrimaryButton.swift
//  IoTROL
//
//  Created by Nikola on 30.05.2023..
//

import Foundation
import SwiftUI

struct PrimaryButton: View {
    var label: String
    var height: CGFloat = 50
    var action: () -> ()
    var disabled: Bool = false
    var body: some View {
        Text(label)
            .font(.system(size: 16, weight: .bold))
            .frame(maxWidth: .infinity)
            .frame(height: height, alignment: .center)
            .background(Color.accent)
            .foregroundColor(.white)
            .cornerRadius(8)
            .onTapGesture(perform: disabled ? {} : action)
    }
}
