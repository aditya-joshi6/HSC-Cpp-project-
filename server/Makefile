CXX = g++
OPENCV= /usr/local/include/opencv4
ONNXRUNTIME= /usr/local/include/onnxruntime

FinalServer: b64ImageDecoder.o b64decodertext.o FinalServer.o inference.o
	$(CXX) -o FinalServer b64decodertext.o b64ImageDecoder.o inference.o FinalServer.o -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_dnn -L/home/shivani/HSC_C++Project/HSC-Cpp-project-/onnxruntime-linux-x64-1.17.3/lib -lonnxruntime

b64ImageDecoder.o: b64ImageDecoder.cpp inference.o
	$(CXX) -c -o b64ImageDecoder.o b64ImageDecoder.cpp

b64decodertext.o: b64decodertext.cpp
	$(CXX) -c b64decodertext.cpp

inference.o: inference.cpp
	$(CXX) -c inference.cpp -I$(OPENCV) -I$(ONNXRUNTIME) -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_dnn -lonnxruntime -o inference.o
clean:
	rm -rvf *.o FinalServer inference
	rm -rvf *.txt *.jpg
