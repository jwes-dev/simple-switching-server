# simple-switching-server
simple switching server in C++. Prefer to use G++ compiler


#  How it works

1. Simple http server
2. Holds the request but starts to send the headers.
3. Mapps the id given in the incoming request in the polling requests to the port number.
4. When a message connection comes in with a endpoint id, it takes the map table and retrieves the port and forwards the message to the port and closes the connection 


### You can modify the code to not close the connection and implement Server-Sent Events Protocol
Use my C++ library to easily implement SSE
The Library allows you to define packets and gives you more control over the connection
