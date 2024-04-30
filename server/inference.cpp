#include <iostream>
#include <vector>
#include <numeric>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

std::string image_path = "/home/shivani/HSC_C++Project/HSC-Cpp-project-/server/output.jpg";
// Preprocess the image
cv::Mat preprocess_image(const std::string& image_path) {
    // Log the image path
    std::cout << "Image Path: " << image_path << std::endl;

    // Read the image
    cv::Mat image = cv::imread(image_path);

    if (image.empty()) {
        std::cerr << "Error: Unable to load image: " << image_path << std::endl;
        return cv::Mat();
    }

    // Log the image dimensions
    std::cout << "Original Image Size: " << image.size() << std::endl;

    cv::resize(image, image, cv::Size(224, 224)); // Resize to match model input size

    // Log the resized image dimensions
    std::cout << "Resized Image Size: " << image.size() << std::endl;

    // Convert pixel values to float and normalize
    image.convertTo(image, CV_32FC3, 1.0 / 255.0); // Normalize pixel values

    // Convert image to blob
    cv::Mat input_image;
    cv::dnn::blobFromImage(image, input_image); // Convert to blob

    return input_image;
}

// Run inference on the image
// Run inference on the image
std::vector<Ort::Value> inference(const std::string& image_path, Ort::Session& session) {
    std::cout << "Running inference..." << std::endl;

    try {
        auto input_tensor = preprocess_image(image_path);
        if (input_tensor.empty()) {
            std::cerr << "Error: Preprocessed input tensor is empty." << std::endl;
            return {}; // Return an empty vector
        }

        std::vector<float> input_data;
        if (input_tensor.isContinuous()) {
            input_data.assign(input_tensor.ptr<float>(), input_tensor.ptr<float>() + input_tensor.total());
        }
        else {
            cv::Mat flattened_input = input_tensor.reshape(1, 1); // Reshape to a 1D matrix
            input_data.assign(flattened_input.ptr<float>(), flattened_input.ptr<float>() + flattened_input.total());
        }

        std::vector<int64_t> input_shape = { 1, 224, 224, 3 }; // assuming NHWC input
        std::vector<const char*> input_names = { "input_1" };
        std::vector<Ort::Value> ort_inputs;
        ort_inputs.reserve(input_names.size());
        ort_inputs.emplace_back(Ort::Value::CreateTensor<float>(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault), input_data.data(), input_data.size(), input_shape.data(), input_shape.size()));
        std::vector<const char*> output_names = { "dense_2" };

        std::cout << "Running session..." << std::endl;
        auto outputs = session.Run(Ort::RunOptions{}, input_names.data(), ort_inputs.data(), input_names.size(), output_names.data(), output_names.size());
       
        std::cout << "Inference completed." << std::endl;
        return outputs;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught during inference: " << e.what() << std::endl;
        return {}; // Return an empty vector
    }
}


int inferenceMain() {

    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test");
    Ort::SessionOptions session_options;
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    std::string model_path = "/home/shivani/HSC_C++Project/HSC-Cpp-project-/ML_modeling/image_model.onnx";
    
    try {
        // Convert model_path to narrow character string
        const char* narrow_model_path = model_path.c_str();
        Ort::Session session(env, narrow_model_path, session_options);
        std::cout << "Starting inference..." << std::endl;
        auto outputs = inference(image_path, session);
        std::vector<float> probabilities;
        probabilities.reserve(outputs[0].GetTensorTypeAndShapeInfo().GetElementCount());
        auto output_data = outputs[0].GetTensorMutableData<float>();
        for (size_t i = 0; i < outputs[0].GetTensorTypeAndShapeInfo().GetElementCount(); ++i) {
            probabilities.push_back(std::exp(output_data[i]) / std::accumulate(output_data, output_data + outputs[0].GetTensorTypeAndShapeInfo().GetElementCount(), 0.0));
        }

        int predicted_class = std::distance(probabilities.begin(), std::max_element(probabilities.begin(), probabilities.end()));
        std::cout << "Predicted class: " << predicted_class << std::endl;
        return predicted_class;
    }
    catch (const std::exception& e) {
        std::cout << "EXCEPTION:  " << e.what() << std::endl;
    }
    return 0;
}
