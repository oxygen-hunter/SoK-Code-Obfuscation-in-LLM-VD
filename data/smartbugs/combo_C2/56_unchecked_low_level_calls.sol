pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    address fuck = 0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D;

    function makeOldFucks(uint32 number) {
        uint32 i = 0;
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if (i >= number) {
                    dispatcher = 2;
                } else {
                    dispatcher = 1;
                }
            } else if (dispatcher == 1) {
                fuck.call(bytes4(sha3("giveBlockReward()")));
                i++;
                dispatcher = 0;
            } else if (dispatcher == 2) {
                break;
            }
        }
    }
}