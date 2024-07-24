# 🖥️ C HTTP Server

Welcome to the C HTTP Server repository! This project implements a basic HTTP server in C, following the RFC rules for HTTP. The server supports multiple concurrent requests, has a connection backlog to queue incoming requests, and provides endpoints for file retrieval and file creation using POST requests.

## 🚀 Features

- **Request Parsing**: The server accurately parses HTTP requests and extracts headers and body.
- **RFC Compliance**: Responses are formatted according to the HTTP RFC rules.
- **Concurrent Requests**: Supports multiple concurrent requests using a connection backlog.
- **Endpoints**:
  - **File Retrieval**: Retrieve the content of a specified file.
  - **File Creation**: Create a file using a POST request with content in the request body.

## 📁 Endpoints

### GET /files/[filename]

Retrieve the content of the specified file.

**Request**:
```sh
GET /files/yourfile.txt HTTP/1.1
```

**Response**:
```sh
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: [file-size]

[file-content]
```

### POST /files/[filename]

Create a file with the specified content.

**Request**:
```sh
POST /files/yourfile.txt HTTP/1.1
Content-Type: text/plain
Content-Length: [content-length]

[file-content]
```

**Response**:
```sh
HTTP/1.1 201 Created
```

## 🛠️ Usage

To compile and run the server:

```sh
gcc -o server server.c
./server
```

## 🔧 Configuration

- **Port**: The server listens on port `4221` by default. Modify the code if a different port is required.
- **Connection Backlog**: The server uses a connection backlog to queue incoming requests. Adjust the backlog size in the code if needed.

## 📜 Code Overview

- **request_parsing.c**: Functions to parse HTTP requests.
- **response_handling.c**: Functions to generate HTTP responses.
- **main.c**: Main server loop, handling incoming connections and dispatching requests.

## 🤝 Contributing

Contributions are welcome! Feel free to submit issues, fork the repository, and send pull requests.

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 🏷️ Credits

Created by ME