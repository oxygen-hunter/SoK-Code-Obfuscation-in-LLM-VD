pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    address[1] f = [0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D];
    
    uint32 j;
     
    function makeOldFucks(uint32 n) {
        for (j = 0; j < n; j++) {
            f[0].call(bytes4(sha3("giveBlockReward()")));
        }
    }
}