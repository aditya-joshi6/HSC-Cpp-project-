#include <cstdlib>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <vector>
#include <math.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <cstring>
#include "b64ImageDecoder.h"
#include "b64decodertext.h"

// Mutex for thread safety
std::mutex mtx;
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
        // Lock MUTEx
        mtx.lock();
        std::ofstream outputFile("request.txt");
        outputFile << request;
        outputFile.close();
        mtx.unlock(); // Unlock the mutex after writing to the file


        // Parse the multipart/form-data body to extract the file data
        boost::beast::http::string_body::value_type requestBody = request.body();

        // Save the request body to a text file
        mtx.lock(); // Lock the mutex 
        std::ofstream outputFile1("request_body.txt", 'wb');
        outputFile1 << requestBody;
        outputFile1.close();
        mtx.unlock(); // Unlock the mutex after writing to the file


        std::vector<std::string> parts;
        boost::algorithm::split(parts, requestBody, boost::algorithm::is_any_of("\n")); // split with "\n" as delimeter
        std::cout << parts[3] << std::endl; //  request body
        int res = b64decoderMain(parts[3]); //
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

void handleTextRequest(http::request<http::string_body>& req) {
    std::string text = req.body();
    if (text.length() != 0) {
        std::string decodedText = decode(text);
        std::cout << "Received text: " << decodedText << std::endl;
	std::string command = "python script.py " + decodedText;
        system(command.c_str()); // invoking python script with decodedText
    }
}

void handleTextConnection(tcp::socket& socket) {
    // Read the request
    boost::beast::flat_buffer buffer;
    http::request<http::string_body> req;
    boost::beast::error_code ec;
    http::read(socket, buffer, req, ec);

    if (ec) {
        // Handle error during read operation
        std::cerr << "Error reading request: " << ec.message() << std::endl;
        return;
    }

    handleTextRequest(req);

    // Reading result predicted by text model
    std::string textResult;
    std::ifstream resultFile("Result.txt");
    while (getline(resultFile, textResult)) {
        std::cout << textResult;
    }
    resultFile.close();

    http::response<http::string_body> res{ http::status::ok, req.version() };
	res.set(http::field::server, "Simple-Cpp-Server");
	res.set(http::field::content_type, "text/plain");
	res.keep_alive(req.keep_alive());
	res.body() = textResult;
	res.prepare_payload();
	res.set(http::field::access_control_allow_origin, "*"); // Allow all origins (for development only)
	res.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
	res.set(http::field::access_control_allow_headers, "Content-Type");
	http::write(socket, res);
}

int main() {
    try {
        boost::asio::io_context ioContext;

        // Start image server on port 8080
        tcp::acceptor acceptor1(ioContext, tcp::endpoint(tcp::v4(), 8080));
        std::thread imageServerThread([&]() {
            while (true) {
                tcp::socket socket(ioContext);
                acceptor1.accept(socket);
                handleFileUpload(socket);
            }
        });

        // Start text server on port 8081
        tcp::acceptor acceptor2(ioContext, tcp::endpoint(tcp::v4(), 8081));
        std::thread textServerThread([&]() {
            while (true) {
                std::cout << "a\n";
                tcp::socket socket(ioContext);
                acceptor2.accept(socket);
                handleTextConnection(socket);
            }
        });

        std::cout << "Servers listening on ports: 8080, 8081" << std::endl;

        imageServerThread.join();
        textServerThread.join();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
