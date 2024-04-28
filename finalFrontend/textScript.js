function displayResults(data) {
  var resultsSection = document.getElementById("resultsSection");
  resultsSection.innerHTML = "<p>" + data + "</p>";
  resultsSection.style.display = "block"; // Make the results section visible
}


function sendText() {
    var text = document.getElementById("textInput").value; 
    if (text.length == 0 ) {
      alert("please input something before submitting");
      throw new Error('Empty text not permitted');
    }
    var base64EncodedText = btoa(text);

    // Configure the request
    fetch("http://localhost:8081", {
      method: "POST",
      headers: {
        "Content-Type": "application/base64" 
      },
      body: base64EncodedText // Send Base64 encoded
    })
    .then(response => {
      if (!response.ok) {
        throw new Error('Network response was not ok');
      }
      return response.text();
    })
    .then(data => {
      displayResults(data);
      console.log(data);
    })
    .catch(error => {
      // handle errors 
      console.error('There was a problem with the fetch operation:', error);
    });
}