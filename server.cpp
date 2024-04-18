#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <boost/algorithm/string.hpp>
#include "b64Decoder.h"

namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;

void handleFileUpload(tcp::socket& socket) {
    boost::beast::flat_buffer buffer;
    http::request<http::string_body> request;

    try {
        // Read the HTTP request
        boost::beast::http::read(socket, buffer, request);
        std::cout << "saving request to file\n" << std::endl;

        // Log the received request details=
        std::cout << "Received HTTP request:\n";
        std::cout << "Method: " << request.method() << "\n";
        std::cout << "Path: " << request.target() << "\n";
        std::cout << "HTTP version: " << request.version() << "\n";
        std::cout << "Content-Type: " << request["Content-Type"] << "\n";

        // Parse the Content-Type header to extract the boundary
        std::string contentType = request["Content-Type"];
        std::string boundary = contentType.substr(contentType.find("boundary=") + 9);

        // Log the extracted boundary
        std::cout << "Boundary: " << boundary << "\n";
        std::ofstream outputFile("request.txt");
        outputFile << request;
        outputFile.close();

        // Parse the multipart/form-data body to extract the file data
        boost::beast::http::string_body::value_type requestBody = request.body();

        // Save the request body to a text file
        std::ofstream outputFile1("request_body.txt", 'wb');
        outputFile1 << requestBody;
        outputFile1.close();

        std::vector<std::string> parts;
        boost::algorithm::split(parts, requestBody, boost::algorithm::is_any_of("\n")); // split with "\n" as delimeter
        std:: cout<< parts[3] << std:: endl; //  request body
        int res = b64decoderMain(parts[3]); // calling decode function

        // Find the part containing the file data
        try {
            http::response<http::string_body> response{ http::status::ok, request.version() };
            response.set(http::field::server, "Boost Beast File Upload Server");
            response.set(http::field::access_control_allow_origin, "*"); // Allow all origins
            //response.body() = "File uploaded successfully";
            response.body() = "Predicted Class: " + std::to_string(res);
            response.prepare_payload();
            boost::beast::http::write(socket, response);
            return;
        }
        catch (const std::exception& e) {
            std::cerr << "Empty file: " << e.what() << "\n";
            http::response<http::string_body> badRequestResponse{ http::status::bad_request, request.version() };
            badRequestResponse.set(http::field::server, "Boost Beast File Upload Server");
            badRequestResponse.set(http::field::access_control_allow_origin, "*"); // Allow all origins
            badRequestResponse.body() = "Invalid request for file upload";
            badRequestResponse.prepare_payload();
            boost::beast::http::write(socket, badRequestResponse);
        }
        
    }

    catch (const std::exception& e) {
        std::cerr << "Error reading HTTP request: " << e.what() << "\n";
        http::response<http::string_body> badRequestResponse{ http::status::bad_request, request.version() };
        badRequestResponse.set(http::field::server, "Boost Beast File Upload Server");
        badRequestResponse.set(http::field::access_control_allow_origin, "*"); // Allow all origins
        badRequestResponse.body() = "Invalid request for file upload";
        badRequestResponse.prepare_payload();
        try {
            boost::beast::http::write(socket, badRequestResponse);
        }
        catch (const std::exception& e) {
            std::cout << "EXCEPTION:  " << e.what() << std::endl;
        }
    }
}

int main() {
    boost::asio::io_context ioContext;
    std::cout << "Server listening at port 8080" << std::endl;
    tcp::acceptor acceptor{ ioContext, { tcp::v4(), 8080 } };
    tcp::socket socket{ ioContext };

    while (true) {
        try {
            acceptor.accept(socket);
        }
        catch (const std::exception& e) {
            std::cout << "EXCEPTION:  " << e.what() << std::endl;
        }
        handleFileUpload(socket);
    }
    return 0;
}