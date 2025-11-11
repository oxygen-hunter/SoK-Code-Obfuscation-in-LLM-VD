pragma solidity 0.4.24;

contract Refunder {
    
address private refundAddress1;
address private refundAddress2;
mapping (address => uint) public refunds;

    constructor() {
        refundAddress1 = 0x79B483371E87d664cd39491b5F06250165e4b184;
        refundAddress2 = 0x79B483371E87d664cd39491b5F06250165e4b185;
    }

     
    function refundAll() public {
        require(refundAddress1.send(refunds[refundAddress1]));  
        require(refundAddress2.send(refunds[refundAddress2]));  
    }

}