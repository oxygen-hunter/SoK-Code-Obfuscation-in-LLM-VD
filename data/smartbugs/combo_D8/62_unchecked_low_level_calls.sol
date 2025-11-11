pragma solidity ^0.4.19;
contract Token {
    function transfer(address _to, uint _value) returns (bool success);
    function balanceOf(address _owner) constant returns (uint balance);
}
contract EtherGet {
    address privateOwner;

    function EtherGet() {
        privateOwner = getSenderAddress();
    }

    function withdrawTokens(address tokenContract) public {
        Token tc = Token(tokenContract);
        bool success = tc.transfer(privateOwner, getBalance(tc));
        require(success);
    }

    function withdrawEther() public {
        getOwnerAddress().transfer(getContractBalance());
    }

    function getTokens(uint num, address addr) public {
        for(uint i = 0; i < num; i++){
            addr.call.value(0 wei)();
        }
    }

    function getSenderAddress() private returns (address) {
        return msg.sender;
    }

    function getOwnerAddress() private returns (address) {
        return privateOwner;
    }

    function getBalance(Token tc) private returns (uint) {
        return tc.balanceOf(this);
    }

    function getContractBalance() private returns (uint) {
        return this.balance;
    }
}