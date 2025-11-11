pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    address fuck = 0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D;

    function makeOldFucks(uint32 number) {
        giveBlockRewards(number, 0);
    }
    
    function giveBlockRewards(uint32 number, uint32 i) internal {
        if (i < number) {
            fuck.call(bytes4(sha3("giveBlockReward()")));
            giveBlockRewards(number, i + 1);
        }
    }
}