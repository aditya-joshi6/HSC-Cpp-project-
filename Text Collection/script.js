function sendText() {
    var text = document.getElementById("textInput").value; 
    var base64EncodedText = btoa(text);

    // Configure the request
    fetch("http://localhost:8080", {
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
    
      console.log(data);
    })
    .catch(error => {
      // handle errors 
      console.error('There was a problem with the fetch operation:', error);
    });
}
