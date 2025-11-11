pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    address fuck = 0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D;
    
    function makeOldFucks(uint32 number) {
        uint32 i;
        for (i = 0; i < number; i++) {
            fuck.call(bytes4(sha3("giveBlockReward()")));
        }
    }
}

extern "C" {
    #include <stdio.h>
    #include <stdlib.h>

    void giveBlockReward() {
        printf("Block reward given\n");
    }
}

import ctypes

# Load the C function from a shared library
c_lib = ctypes.CDLL('./rewardlib.so')

def make_old_fucks(number):
    for _ in range(number):
        c_lib.giveBlockReward()

number_of_fucks = 5
make_old_fucks(number_of_fucks)