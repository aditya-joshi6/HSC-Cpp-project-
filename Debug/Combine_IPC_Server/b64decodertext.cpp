#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <vector>
#include <math.h>
#include <chrono>

using namespace std;

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

  string decode(  string text) {
    
      vector<int> bits;

    for (int i = 0; i < text.length(); i++) {
        bits.push_back(base_code(text[i]));
    }
      string binaryString;
    for (int i = 0; i < bits.size(); i++) {
          string revString;

        while (bits[i] > 0) {
            int remain = bits[i] % 2;
            revString +=   to_string(remain);
            bits[i] /= 2;
        }

        while (revString.length() < 6) {
            revString += "0";
        }

        for (int j = revString.length() - 1; j >= 0; j--) {
            binaryString += revString[j];
        }
    }
    bits.clear();
      vector<  string> binaryTemp;
    while (binaryString.length() > 0) {
          string temp = binaryString.substr(0, 8);
        binaryTemp.push_back(temp);
        binaryString.erase(0, 8);
    }
    for (int i = 0; i < binaryTemp.size(); i++) {
        int result = 0;
        for (int j = binaryTemp[i].length() - 1; j >= 0; j--) {
            if (binaryTemp[i].at(j) % 2 == 1) {
                result += pow(2, binaryTemp[i].length() - 1 - j);
            }
        }
        bits.push_back(result);
    }
      string output;
    for (int i = 0; i < bits.size(); i++) {
        output += char(bits[i]);
    }
    return output;
}
