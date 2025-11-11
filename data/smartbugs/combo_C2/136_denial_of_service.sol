pragma solidity 0.4.24;

contract Refunder {
    
address[] private refundAddresses;
mapping (address => uint) public refunds;

    constructor() {
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b184);
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b185);
    }

    function refundAll() public {
        uint x = 0;
        uint state = 0;
        bool running = true;
        
        while (running) {
            if (state == 0) {
                if (x < refundAddresses.length) {
                    state = 1;
                } else {
                    running = false;
                }
            } else if (state == 1) {
                require(refundAddresses[x].send(refunds[refundAddresses[x]]));
                x++;
                state = 0;
            }
        }
    }
}