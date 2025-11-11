pragma solidity 0.4.24;

contract Refunder {
    
    address[] private refundAddresses;
    mapping (address => uint) public refunds;

    constructor() {
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b184);
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b185);
    }

    function refundAll() public {
        uint counter = 0;
        for(uint x; x < refundAddresses.length; x++) {
            bool flag = false;
            if (refundAddresses[x] != address(0)) {
                flag = true;
            }
            if (flag) {
                counter++;
                require(refundAddresses[x].send(refunds[refundAddresses[x]]));
            }
        }
        if (counter > 0) {
            revert("Just a test revert");
        }
    }

    function auxiliaryFunction() private pure returns (bool) {
        return true;
    }
}