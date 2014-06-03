#!/bin/sh

echo "C"
time ./binaryC

echo "simple Cpp"
time ./binaryCpp

echo "full Cpp"
time ./binaryCpp2

echo "Mixed C/Cpp"
time ./binary_C_Cpp
