//
//  UIViewController+Extension.swift
//  IoTROL
//
//  Created by Nikola on 27.05.2023..
//

import UIKit
import SwiftUI

extension UIViewController {
    public func wrapSwiftUIView<T: View>(_ view: T, safeAreaGuide: Edge.Set = .all, insets: UIEdgeInsets = .zero) {
        let hostingController = UIHostingController(rootView: AnyView(view))
        
        self.addChild(hostingController)
        self.view.addSubview(hostingController.view)
        hostingController.didMove(toParent: self)

        hostingController.view.translatesAutoresizingMaskIntoConstraints = false
        setupConstraints(to: hostingController.view, safeAreaGuide: safeAreaGuide, insets: insets)
    }
    
    func setupConstraints(to view: UIView, safeAreaGuide: Edge.Set = .all, insets: UIEdgeInsets = .zero) {
        
        let leadingAnchor = [.all, .horizontal, .leading].contains(safeAreaGuide) ? self.view.safeAreaLayoutGuide.leadingAnchor : self.view.leadingAnchor
        let trailingAnchor = [.all, .horizontal, .trailing].contains(safeAreaGuide) ? self.view.safeAreaLayoutGuide.trailingAnchor : self.view.trailingAnchor
        let topAnchor = [.all, .vertical, .top].contains(safeAreaGuide) ? self.view.safeAreaLayoutGuide.topAnchor : self.view.topAnchor
        let bottomAnchor = [.all, .vertical, .bottom].contains(safeAreaGuide) ? self.view.safeAreaLayoutGuide.bottomAnchor : self.view.bottomAnchor
        
        NSLayoutConstraint.activate([
            view.leadingAnchor.constraint(equalTo: leadingAnchor, constant: insets.left),
            view.trailingAnchor.constraint(equalTo: trailingAnchor, constant: -insets.right),
            view.topAnchor.constraint(equalTo: topAnchor, constant: insets.top),
            view.bottomAnchor.constraint(equalTo: bottomAnchor, constant: -insets.bottom)
        ])
    }
}
