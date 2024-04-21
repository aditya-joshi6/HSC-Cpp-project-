#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include<cstdlib>
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
#include "D:\repos\inference\b64decoder.h"
#include <cstring>

// Mutex for thread safety
std::mutex mtx;
namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;
// IPC
using boost::interprocess::read_write;
using boost::interprocess::open_or_create;
using namespace boost::interprocess;

// Base64 Alphabet char function
int base_code(char chr) {
    if (chr == '=') {
        return NULL;
    }

    char alphabet[] = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

    int len = sizeof(alphabet) / sizeof(char);

    for (int i = 0; i < len; i++) {
        if (chr == alphabet[i]) {
            return i;
        }
    }
}

std::string decode(std::string text) {
    // Declaration
    std::vector<int> bits;

    // Get each character Base64 code and put it in bits vector
    for (int i = 0; i < text.length(); i++) {
        bits.push_back(base_code(text[i]));
    }

    // Turn Base64 code into binary
    std::string binaryString;
    for (int i = 0; i < bits.size(); i++) {
        std::string revString;

        // Calculate Binary
        while (bits[i] > 0) {
            int remain = bits[i] % 2;
            revString += std::to_string(remain);
            bits[i] /= 2;
        }

        // If Binary length is less than 6, put a 0 behind it
        while (revString.length() < 6) {
            revString += "0";
        }

        // Reverse the string so it would match before we divide it to a group of 8
        for (int j = revString.length() - 1; j >= 0; j--) {
            binaryString += revString[j];
        }
    }
    // Clear bits vector so we could use it later
    bits.clear();
    // Divide the binary into a group with 8 elements
    std::vector<std::string> binaryTemp;
    while (binaryString.length() > 0) {
        std::string temp = binaryString.substr(0, 8);
        binaryTemp.push_back(temp);
        binaryString.erase(0, 8);
    }
    // Convert from Binary to ASCII code
    for (int i = 0; i < binaryTemp.size(); i++) {
        int result = 0;
        for (int j = binaryTemp[i].length() - 1; j >= 0; j--) {
            if (binaryTemp[i].at(j) % 2 == 1) {
                result += pow(2, binaryTemp[i].length() - 1 - j);
            }
        }
        // Put the ASCII into the bits vector
        bits.push_back(result);
    }
    // We convert the ASCII to char
    std::string output;
    for (int i = 0; i < bits.size(); i++) {
        output += char(bits[i]);
    }
    // Return the output
    return output;
}
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
    std::string decodedText = decode(text);

    if (decodedText.length() != 0) {
        std::cout << "Received text: " << decodedText << std::endl;

        // IPC Section
        windows_shared_memory shm_obj(create_only, "shared_memory", read_write, 1000);
        std::cout << "Shared Memory Name: " << shm_obj.get_name() << std::endl;

        offset_t size = shm_obj.get_size();
        std::cout << "Size of share memory: " << size << std::endl;

        //Map the whole shared memory in this process
        mapped_region region(shm_obj, read_write);

        const char* message = decodedText.c_str();
        const size_t message_size = std::strlen(message);
        const size_t buffer_size = region.get_size();
        try {
            if (message_size + 1 <= buffer_size) {  
                std::memcpy(static_cast<char*>(region.get_address()), message, message_size + 1);
            }
        }
        catch (...) {
            std::cout << "Message not written";
        }

        system("python script.py");
        char* mem = static_cast<char*>(region.get_address());
        std::cout << "Data in shared memory written by Python script: " << mem << "\n";
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

    http::response<http::string_body> res{ http::status::ok, req.version() };
	res.set(http::field::server, "Simple-Cpp-Server");
	res.set(http::field::content_type, "text/plain");
	res.keep_alive(req.keep_alive());
	res.body() = "Text received successfully";
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
