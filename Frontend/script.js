document.addEventListener('DOMContentLoaded', function() {
    const uploadInput = document.getElementById('image-upload');
    const detectButton = document.getElementById('detect-button');
    const resultContainer = document.getElementById('result-container');
    const resultSection = document.querySelector('.result-section');

    // Function to display loading animation
    function showLoading() {
        resultContainer.innerHTML = '<div class="loader"></div>';
        resultSection.style.display = 'block';
    }

    // Function to hide loading animation
    function hideLoading() {
        resultContainer.innerHTML = ''; // Clear result container
    }

    // Event listener for file upload
    uploadInput.addEventListener('change', function(event) {
        const file = event.target.files[0];
        if (file) {
            const reader = new FileReader();
            reader.onload = function(e) {
                const imageData = e.target.result;
                // Display uploaded image (optional)
                // const img = document.createElement('img');
                // img.src = imageData;
                // resultContainer.appendChild(img);
            };
            reader.readAsDataURL(file);
        }
    });

    // Event listener for detection button
    detectButton.addEventListener('click', function() {
        const file = uploadInput.files[0];
        if (file) {
            showLoading(); // Show loading animation
            // Simulate backend call (replace with actual backend API call)
            setTimeout(function() {
                // Simulated detection result
                const result = {
                    detected: true,
                    message: 'Disease detected: COVID-19'
                };
                // Display detection result
                resultContainer.innerHTML = `<p>${result.message}</p>`;
            }, 2000); // Simulated delay (2 seconds)
        } else {
            alert('Please upload an image first.');
        }
    });
});
