#!/bin/bash
rm -f *.out
g++ -o server.out server_example.cpp socket.cpp
g++ -o client.out client_example.cpp socket.cpp
