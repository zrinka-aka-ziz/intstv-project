//
//  BlindsViewController.swift
//  IoTROL
//
//  Created by Nikola on 30.05.2023..
//

import Foundation
import UIKit

public class BlindsViewController: UIViewController {
    private var blinds: BlindsModel
    
    public init(blinds: BlindsModel) {
        self.blinds = blinds
        
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    public override func viewDidLoad() {
        super.viewDidLoad()
        self.view.backgroundColor = .background
        
        
        let viewModel = BlindsView.ViewModel(blinds: blinds) { [weak self] value in
            self?.confirmChange(value)
        }
        wrapSwiftUIView(BlindsView(viewModel: viewModel))
    }
    
    private func confirmChange(_ value: Double) {
        dismiss(animated: true)
    }
}
