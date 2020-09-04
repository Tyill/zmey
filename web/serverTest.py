from flask import Flask
app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello, Wo rld!'


# // eslint-disable-next-line no-undef
# const express = require("express");

# const app = express();

# app.use(express.static(__dirname + "/js"));

# app.use(function(request, response){
  
#   let path = request.path;
#   if (path == '/') path = "/index.html";

#    response.sendFile(__dirname + path);
# });

# app.listen(3000);