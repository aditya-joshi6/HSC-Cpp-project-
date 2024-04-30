CXX = g++
OPENCV= /usr/local/include/opencv4
ONNXRUNTIME= /usr/local/include/onnxruntime
CXXFLAGS = -I$(OPENCV) -I$(ONNXRUNTIME)
ONNXRUNTIMELOCATION= /home/shivani/HSC_C++Project/HSC-Cpp-project-/onnxruntime-linux-x64-1.17.3/lib -lonnxruntime
LIBS= -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_dnn
OBJS= b64ImageDecoder.o b64decodertext.o FinalServer.o inference.o

FinalServer: $(OBJS)
	$(CXX) -o FinalServer $(OBJS) $(LIBS) -L$(ONNXRUNTIMELOCATION)

b64ImageDecoder.o: b64ImageDecoder.cpp inference.o
	$(CXX) -c -o b64ImageDecoder.o b64ImageDecoder.cpp

b64decodertext.o: b64decodertext.cpp
	$(CXX) -c b64decodertext.cpp

inference.o: inference.cpp
	$(CXX) -c inference.cpp $(CXXFLAGS) $(LIBS) -lonnxruntime -o inference.o
clean:
	rm -rvf *.o FinalServer inference
	rm -rvf *.txt *.jpg
