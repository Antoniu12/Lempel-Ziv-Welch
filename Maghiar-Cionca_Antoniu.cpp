#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

vector<int> compression(Mat_<uchar>& img) {
    map<string, int> dictionary;
    vector<int> LZW;
    int dictSize = 256;
    for (int i = 0; i < 256; i++) {
        dictionary[string(1, char(i))] = i;
    }

    string w;
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            char pixelValue = (char)(img(i, j));
            string aux = w + pixelValue;

            if (dictionary.count(aux)) {
                w = aux;
            }
            else {
                LZW.push_back(dictionary[w]);
                dictionary[aux] = dictSize++;
                w = string(1, pixelValue);
            }
        }
    }

    if (!w.empty()) {
        LZW.push_back(dictionary[w]);
    }

    return LZW;
}

Mat_<uchar> decompression(const vector<int>& compressedData, int rows, int cols) {
    map<int, string> dictionary;
    int dictSize = 256;

    for (int i = 0; i < 256; i++) {
        dictionary[i] = string(1, char(i));
    }

    string w(1, char(compressedData[0]));
    string result = w;

    //////MODIFICA
    for (size_t i = 1; i < compressedData.size(); i++) {
        int k = compressedData[i];
        string entry;
        if (dictionary.count(k)) {
            entry = dictionary[k];
        }
        else if (k == dictSize) {
            entry = w + w[0];
        }

        result += entry;
        dictionary[dictSize++] = w + entry[0];
        w = entry;
    }
    ///MODIFICA
    Mat_<uchar> dst(rows, cols);
    int k = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            dst(i, j) = result[k++];
        }
    }

    return dst;
}

void writeCompressedDataToFile(vector<int>& compressedData, int rows, int cols, const string& filename) {
    ofstream g(filename, ios::binary);
    if (!g) {
        cout << "Cannot open the file to write!" << endl;
        return;
    }

    g.write((char*)(&rows), sizeof(int));
    g.write((char*)(&cols), sizeof(int));

    for (int code : compressedData) {
        g.write((char*)(&code), sizeof(int));
    }

    g.close();
}

vector<int> readCompressedDataFromFile(const string& filename, int& rows, int& cols) {
    ifstream f(filename, ios::binary);
    if (!f) {
        cout << "Cannot open the file to read!" << endl;
        exit(1);
    }

    f.read((char*)(&rows), sizeof(int));
    f.read((char*)(&cols), sizeof(int));

    vector<int> compressedData;
    int code;
    while (f.read((char*)(&code), sizeof(int))) {
        compressedData.push_back(code);
    }

    f.close();
    return compressedData;
}

int main() {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);

    Mat_<uchar> img = imread("Images/saturn.bmp", 0);

    vector<int> compressedData = compression(img);

    string filename = "compressed_data.bin";
    writeCompressedDataToFile(compressedData, img.rows, img.cols, filename);

    int rows, cols;
    vector<int> readCompressedData = readCompressedDataFromFile(filename, rows, cols);
    Mat_<uchar> decompressedImage = decompression(readCompressedData, rows, cols);

    int originalSize = img.rows * img.cols * 8;
    int compressedSize = compressedData.size() * sizeof(int) * 8;

    cout << "Original Size: " << originalSize << " Bytes" << endl;
    cout << "Compressed Size: " << compressedSize << " Bytes" << endl;
    cout << "Compression Ratio: " << (double)originalSize / compressedSize << endl;


    imshow("Original Image", img);
    imshow("Decompressed Image", decompressedImage);
    waitKey(0);

    return 0;
}