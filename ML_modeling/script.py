import sys
import onnxruntime as ort
import numpy as np
import re
import openai
from openai import OpenAI

def preprocess_text(text):
    text = text.lower()
    text = re.sub(r'[^a-zA-Z0-9\s]', '', text)  # Remove non-alphanumeric characters
    return text.strip()

def fetch_embeddings(text):
    clean_text= preprocess_text(text)
    response = client.embeddings.create(input=clean_text, model='text-embedding-3-small')
    embeddings = response.data[0].embedding
    return embeddings


def load_onnx_model(model_path):
    return ort.InferenceSession(model_path)

def run_inference(model, embedded_text):

    # Create model input from the preprocessed text (example: use a simple numeric representation)
    input_data = np.array([embedded_text]).astype(np.float32)  # Example: Input as word count

    # Run inference
    input_name = model.get_inputs()[0].name
    output_name = model.get_outputs()[0].name
    result = model.run([output_name], {input_name: input_data})

    # Return the inference result
    return result

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python inference_script.py 'input_text'")
        sys.exit(1)

    input_text = sys.argv[1]
    api_key = ''
    client = OpenAI(api_key=api_key)
    embed = fetch_embeddings(input_text)    

    # Path to the ONNX model file
    model_path = r'C:\Users\Work\Desktop\cppCode\project\text_model.onnx'

    # Load the ONNX model
    model = load_onnx_model(model_path)

    # Perform inference on the input text
    inference_result = run_inference(model, embed)

    # Print or process the inference result as needed
    res = inference_result[0][0][0]
    result_binary = 'Positive'
    if res < 0.5:
        result_binary = 'Negative'
    conf = 2 * abs(0.5 - res)
    fr = str(result_binary) + " " + str(conf)
    file1 = open("Result.txt","w")
    file1.write(fr)
    file1.close()
    print("Inference Result:", result_binary, conf)
