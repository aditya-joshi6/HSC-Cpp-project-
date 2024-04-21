#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <math.h>
#include <chrono>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

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
void handle_request(http::request<http::string_body>& req) {
	// Extract the text from the request
	std::string text = req.body();
	std::string decodedText = decode(text);
	std::cout << "Received text: " << decodedText << std::endl;
}
void handle_connection(tcp::socket& socket) {
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
	// Handle the request
	handle_request(req);
	// Send response with CORS headers
	http::response<http::string_body> res{ http::status::ok, req.version() };
	res.set(http::field::server, "Simple-Cpp-Server");
	res.set(http::field::content_type, "text/plain");
	res.keep_alive(req.keep_alive());
	res.body() = "Text received successfully";
	res.prepare_payload();
	// Correct CORS header names
	res.set(http::field::access_control_allow_origin, "*"); // Allow all origins (for development only)
	res.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
	res.set(http::field::access_control_allow_headers, "Content-Type");

	http::write(socket, res);
}
void start_server(boost::asio::io_context& io_context, unsigned short port) {
	tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

	// Print message when server starts listening
	std::cout << "Server listening on port: " << port << std::endl;

	while (true) {
		tcp::socket socket(io_context);
		acceptor.accept(socket);
		handle_connection(socket);
	}
}
int main() {
	try {
		boost::asio::io_context io_context;
		start_server(io_context, 8081);
		io_context.run();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}