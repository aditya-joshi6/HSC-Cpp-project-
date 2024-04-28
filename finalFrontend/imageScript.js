const loader = document.getElementById('loader');
const toggleBtn = document.getElementById('toggleBtn');
const navbar = document.getElementById('navbar');

toggleBtn.addEventListener('click', function() {
    navbar.classList.toggle('active');
});

async function uploadImage() {
    const fileInput = document.getElementById('fileInput');
    const file = fileInput.files[0];
    
    if (!file) {
        alert('Please select an image file.');
        return;
    }
    
    if(file.size > 1024000) {
        alert("File is too big!");
        this.value = "";
        return;
    }
    
    // Show loader when uploading starts
    loader.style.display = 'block';
    
    const reader = new FileReader();
    reader.onload = async function(e) {
        const base64Image = e.target.result.split(',')[1];
        
        try {
            // Display image preview
            displayImagePreview(e.target.result);
            
            // Send data to backend
            await sendDataToBackend(base64Image);
            console.log('Image uploaded successfully!');
        } catch (error) {
            console.error('Error uploading image:', error);
            alert('Failed to upload image. Please try again.');
        } finally {
            // Hide loader when upload completes (whether successful or not)
            loader.style.display = 'none';
        }
    };
    
    reader.readAsDataURL(file);
}
async function sendDataToBackend(base64Image) {
    const url = 'http://localhost:8080';

    const formData = new FormData();
    formData.append('image', base64Image);

    const requestOptions = {
        method: 'POST',
        body: formData
    };

    const response = await fetch(url, requestOptions);
    const responseData = await response.text(); 
    displayResults(responseData);
    
    console.log(response);
    if (!response.ok) {
        throw new Error(`HTTP error! Status: ${response.status}`);
    }
    //resultContainer.innerHTML = `<p>${result.message}</p>`;
}

function displayImagePreview(imageData) {
    const imagePreview = document.getElementById('imagePreview');
    imagePreview.innerHTML = `<img src="${imageData}" alt="Uploaded Image" />`;
}

function displayResults(data) {
    var resultsSection = document.getElementById("resultsSection");
    resultsSection.innerHTML = "<p>" + data + "</p>";
    resultsSection.style.display = "block"; // Make the results section visible
  }