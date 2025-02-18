# Web Server in C

A simple web server implemented in C.

## Compilation

Navigate to the `server` directory and compile using:

```sh
gcc main.c Server.c -o a -lws2_32
```

## Running the Server

Execute the compiled binary:
```sh
./a
```

## On Web Browser

Open the web brwoser and run

```sh
http://localhost:8082
```

## Requirements
- GCC compiler
- Windows environment (WS2_32 library required)

## Notes
Ensure that necessary ports are available before running the server.

---
***© 2025 Sandhavi Wanigasooriya. All rights reserved.***