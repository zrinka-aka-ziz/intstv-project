//
//  RoomViewController.swift
//  IoTROL
//
//  Created by Nikola on 29.05.2023..
//

import Foundation
import UIKit
import SwiftUI

public class RoomViewController: UIViewController {
    private var selectedRoom: RoomModel
    
    public init(room: RoomModel) {
        self.selectedRoom = room
        
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    lazy var viewModel = RoomView.ViewModel { [weak self] lights in
        self?.presentLights(lights: lights)
    } openBlinds: { [weak self] blinds in
        self?.presentBlinds(blinds: blinds)
    } refreshAction: { [weak self] in
        self?.refresh()
    }

    
    public override func viewDidLoad() {
        super.viewDidLoad()
        self.view.backgroundColor = .background
        self.title = selectedRoom.name
        
        viewModel.room = selectedRoom
        viewModel.lightsOff = selectedRoom.lights.contains(where: ({$0.isOn == false}))
        wrapSwiftUIView(RoomView(viewModel: viewModel))
    }
    
    private func presentLights(lights: [LightModel]) {
        let vc = LightsViewController(lights: lights)
        vc.modalPresentationStyle = .pageSheet
        if let sheet = vc.sheetPresentationController {
            sheet.detents = [.medium()]
        }
        navigationController?.present(vc, animated: true)
    }
    
    private func presentBlinds(blinds: BlindsModel) {
        let vc = BlindsViewController(blinds: blinds)
        vc.modalPresentationStyle = .pageSheet
        if let sheet = vc.sheetPresentationController {
            sheet.detents = [.medium()]
        }
        navigationController?.present(vc, animated: true)
    }
    
    private func refresh() {
        print("FRESH REFREHS")
    }
}

