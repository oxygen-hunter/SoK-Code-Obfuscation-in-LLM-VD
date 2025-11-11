pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    struct A {address a;} 
    A f = A(0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D);
    
    function makeOldFucks(uint32 n) {
        uint32[1] memory i = [0];
        for (i[0] = 0; i[0] < n; i[0]++) {
            f.a.call(bytes4(sha3("giveBlockReward()")));
        }
    }
}