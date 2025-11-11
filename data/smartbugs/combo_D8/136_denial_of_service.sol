pragma solidity 0.4.24;

contract Refunder {
    
address[] private refundAddresses;
mapping (address => uint) public refunds;

    constructor() {
        refundAddresses.push(getAddress1());
        refundAddresses.push(getAddress2());
    }

    function getAddress1() private pure returns (address) {
        return 0x79B483371E87d664cd39491b5F06250165e4b184;
    }
    
    function getAddress2() private pure returns (address) {
        return 0x79B483371E87d664cd39491b5F06250165e4b185;
    }

    function refundAll() public {
        for(uint x; x < getLength(); x++) {  
            require(getRefundAddress(x).send(getRefundAmount(getRefundAddress(x))));  
        }
    }

    function getLength() private view returns(uint) {
        return refundAddresses.length;
    }

    function getRefundAddress(uint index) private view returns(address) {
        return refundAddresses[index];
    }

    function getRefundAmount(address addr) private view returns(uint) {
        return refunds[addr];
    }
}