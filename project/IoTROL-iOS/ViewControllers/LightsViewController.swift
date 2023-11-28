//
//  LightsViewController.swift
//  IoTROL
//
//  Created by Nikola on 30.05.2023..
//

import Foundation
import UIKit

public class LightsViewController: UIViewController {
    private var lights: [LightModel]
    
    public init(lights: [LightModel]) {
        self.lights = lights
        
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    public override func viewDidLoad() {
        super.viewDidLoad()
        self.view.backgroundColor = .background
        
        let viewModel = LightsView.ViewModel(lights: lights) { [weak self] lights in
            self?.confirmAction(lights)
        }
        wrapSwiftUIView(LightsView(viewModel: viewModel))
    }
    
    private func confirmAction(_ lights: [LightModel]) {
        dismiss(animated: true)
    }
}
