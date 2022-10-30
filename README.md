# TCP Trading Server

## Overview

The Trading server (`server.cpp`) can process and execute trade orders across multiple traders (`client.cpp`) on the platform. It also supports operations from the clients such as adding, modifying or deleting any order on a financial instrument, viewing all orders placed for an instrument, etc.

## Usage

 -  Download this repository, or specifically the files `server.cpp` and `client.cpp`.

 -  Open the terminal in the repository location. Compile `server.cpp` executable file to `server` and run it. This runs the server on port `5555` (can be changed), and the client connects to this port.
```bat
$ g++ server.cpp -o server
$ ./server
```

 -  Compile the `client.cpp` executable file to `client` and run it in a separate terminal instance. At this point, our risk server and the client have established a connection and the client can process requests.
```bat
$ g++ client.cpp -o client
```

 - This client executable can be run across any number of terminal instances and they connect to the server.
 ```bat
 $ ./client
 ```

  - Alternatively, you could download all the files in the repository and run the bash script `execute.sh`, followed by the file name (`server.cpp` or `client.cpp`) and it runs the executable of it.

 ```bat
 $ ./execute.sh server.cpp
 ```
 ```bat
 $ ./execute.sh client.cpp
 ```

