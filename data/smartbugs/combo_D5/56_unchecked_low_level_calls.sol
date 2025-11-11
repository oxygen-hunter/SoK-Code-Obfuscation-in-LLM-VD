pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    address f1 = 0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D;
    
    function makeOldFucks(uint32 n1) {
        uint32 i1;
        for (i1 = 0; i1 < n1; i1++) {
            f1.call(bytes4(sha3("giveBlockReward()")));
        }
    }
}