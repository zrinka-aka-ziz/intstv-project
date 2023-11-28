//
//  NetworkClient.swift
//  IoTROL
//
//  Created by Nikola on 01.06.2023..
//

import Foundation
import Alamofire
import Combine

public enum HTTPError: LocalizedError {
    case statusCode
    case parametersSerialization
    case unknown
}

public typealias DecodableError = Error & Decodable

public class NetworkClient {
    public static let instance: NetworkClient = .init()
    
    public static var baseURL: String = "baseURL"
    public static var token: String = "TOKEN"
    private var header: HTTPHeader = HTTPHeader(name: "Authorization", value: "Bearer \(token)")
    
    private init() {}
    
    public func request<ValueType: Decodable, ErrorType: DecodableError, P: Encodable>(endpoint: String, method: String, parameters: P, encoder: ParameterEncoder = JSONParameterEncoder.default, decoder: JSONDecoder = JSONDecoder()) -> AnyPublisher<Result<ValueType, ErrorType>, Error> {
        let url = URL(string: NetworkClient.baseURL)!.appendingPathComponent(endpoint)
        
        return AF
            .request(
                url,
                method: HTTPMethod(rawValue: method),
                parameters: parameters,
                encoder: encoder,
                headers: [header]
            )
            .validate(contentType: ["application/json", "application/problem+json", "application/json; charset=UTF-8"])
            .publishData()
            .tryMap { response in
                try Self.decode(
                    response: response,
                    decoder: decoder
                )
            }
            .eraseToAnyPublisher()
    }
    
    public static func decode<ValueType: Decodable, ErrorType: DecodableError>(response: AFDataResponse<Data>, decoder: JSONDecoder) throws -> Result<ValueType, ErrorType> {
       
        switch response.result {
        case .success:
            guard
                let httpResponse = response.response
            else { throw HTTPError.unknown }
            
            let statusCode = httpResponse.statusCode
            let data = response.data
            
            switch statusCode {
            case 200..<300:
                let value = try decoder.decode(ValueType.self, from: data!)
                
                return .success(value)
            case 400..<500:
                do {
                    let body = try decoder.decode(ValueType.self, from: data!)
                    return .success(body)
                } catch {
                    
                }
                let error = try decoder.decode(ErrorType.self, from: data!)
                return .failure(error)
            default:
                throw response.error!
            }
        case let .failure(error):
            throw error
        }
    }
}
