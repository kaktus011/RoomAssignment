# RoomAssignment

RoomAssignment is a multithreaded Monte Carlo simulation for solving the Room Assignment Problem. It assigns students to rooms while minimizing penalties for exceeding room capacities and placing incompatible students in the same room.
Please note that this is made to be used for experimentation on Grid5000 machines.

## Features
- Multithreaded Monte Carlo simulation for efficient computation.
- Configurable number of threads, rooms, and students.
- Evaluates solutions based on room capacity and incompatibility constraints.

## Default Configuration
- **Threads**: 4  
- **Rooms**: 10 (each with a capacity of 10)  
- **Students**: 100  

## Usage
1. Compile the program:
   ```sh
   g++ -std=c++17 -pthread -o RoomAssignment source/main.cpp