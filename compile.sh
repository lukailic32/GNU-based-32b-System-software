#!/bin/bash
g++ -o assembler ./src/mainAssembler.cpp ./src/assembler.cpp ./src/hex.cpp

g++ -o linker ./src/mainLinker.cpp ./src/linker.cpp ./src/hex.cpp

g++ -o emulator ./src/mainEmulator.cpp ./src/emulator.cpp ./src/hex.cpp