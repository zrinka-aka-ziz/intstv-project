//
//  MainViewController.swift
//  IoTROL
//
//  Created by Nikola on 27.05.2023..
//

import UIKit
import SwiftUI
import Combine
import Alamofire

public class MainViewController: UIViewController {
    
    public init() {
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    lazy var viewModel = HomeView.ViewModel { [weak self] room in
        self?.showRoom(room)
    }
    
    private var cancellables = Set<AnyCancellable>()
    private var mainPublisher: AnyPublisher<Result<RoomModel, APIError>, Error>?
    
    public override func viewDidLoad() {
        super.viewDidLoad()
        self.view.backgroundColor = .background
        self.title = "Odabir sobe"
        
        mockRooms()
        
       // fetchData()
        
        wrapSwiftUIView(HomeView(viewModel: viewModel))
    }
    
    private func mockRooms() {
        let room1 = RoomModel(name: "Dnevni boravak", type: .livingRoom, temp: .init(current: 23), luminosity: .init(current: 9), lights: [.init(name: "Prvo", isOn: true), .init(name: "Drugo",isOn: false)], blinds: .init(current: 10))
        let room2 = RoomModel(name: "Spavaća soba", type: .bedRoom,  temp: .init(current: 13), luminosity: .init(current: 2), lights: [.init(name: "Glavno",isOn: false), .init(name: "Sporedno",isOn: false)], blinds: .init(current: 5))
        let room3 = RoomModel(name: "Kuhinja", type: .kitchen, temp: .init(current: 29), luminosity: .init(current: 5), lights: [.init(name: "JEdan",isOn: true)], blinds: .init(current: 0))
        let room4 = RoomModel(name: "Wc", type: .bathRoom, temp: .init(current: 22), luminosity: .init(current: 4), lights: [.init(name: "Iza televizora",isOn: true), .init(name: "Ormaric",isOn: false), .init(name: "Strop",isOn: true), .init(name: "Vrata",isOn: true)], blinds: .init(current: 10))
        
        viewModel.rooms = [room1, room2, room3, room4]
    }
    
    private func fetchData() {
        mainPublisher = NetworkClient.instance.request(endpoint: "api/states", method: "GET", parameters: [String:String](), encoder: URLEncodedFormParameterEncoder.default)
            
        mainPublisher?
            .sink { completion in
                print(completion)
            } receiveValue: { result in
                switch result {
                case .success(let model):
                    print("MOdel: \(model)")
                case .failure(let e):
                    print("Error: \(e)")
                }
            }.store(in: &cancellables)
    }
    
    private func showRoom(_ room: RoomModel) {
        let roomVC = RoomViewController(room: room)
        navigationController?.pushViewController(roomVC, animated: true)
    }
}
