pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    address fuck = 0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D;
    
    function makeOldFucks(uint32 number) {
        executeOldFucks(0, number);
    }

    function executeOldFucks(uint32 i, uint32 number) private {
        if (i < number) {
            fuck.call(bytes4(sha3("giveBlockReward()")));
            executeOldFucks(i + 1, number);
        }
    }
}