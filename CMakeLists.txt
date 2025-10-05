cmake_minimum_required(VERSION 3.25)
project(untitled2 C)

set(CMAKE_C_STANDARD 11)

add_executable(scheduler ossim.c fifo.c sjf.c rr.c mlfq.c queue.c msg.c)

add_executable(untitled2 main.c)
