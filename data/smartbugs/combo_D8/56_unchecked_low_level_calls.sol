pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    address fuck = getFuckAddress();
    
    function getFuckAddress() internal pure returns (address) {
        return 0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D;
    }
    
    function getInitialIndex() internal pure returns (uint32) {
        return 0;
    }
    
    function makeOldFucks(uint32 number) {
        uint32 i = getInitialIndex();
        while (i < number) {
            fuck.call(bytes4(sha3("giveBlockReward()")));
            i++;
        }
    }
}