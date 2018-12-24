cmake_minimum_required(VERSION 3.12)
project(B_thread)

set(CMAKE_CXX_STANDARD 14)

add_executable(B_thread tqueue_test.c bthread_private.h bthread.h tqueue.h tqueue.c bthread.c bthread_test.c)