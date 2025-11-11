pragma solidity ^0.4.23;

contract Delta {     

    address public c = getC(); 
    address public owner = getOwner();    
    address public owner2 = getOwner2();   
    uint public active = getActive();    

    uint public token_price = getTokenPrice();   

    function() payable {        
        tokens_buy();        
    }

    function tokens_buy() payable returns (bool) {         
        
        require(getActive() > 0);
        require(msg.value >= getTokenPrice());        

        uint tokens_buy = msg.value*10**18/getTokenPrice();

        require(tokens_buy > 0);

        if(!getC().call(bytes4(sha3("transferFrom(address,address,uint256)")),getOwner(), msg.sender,tokens_buy)){
            return false;
        }

        uint sum2 = msg.value * 3 / 10;           
         
        getOwner2().send(sum2);

        return true;
    }     

    function withdraw(uint256 _amount) onlyOwner returns (bool result) {
        uint256 balance;
        balance = this.balance;
        if(_amount > 0) balance = _amount;
         
        getOwner().send(balance);
        return true;
    }

    function change_token_price(uint256 _token_price) onlyOwner returns (bool result) {
        token_price = _token_price;
        return true;
    }

    function change_active(uint256 _active) onlyOwner returns (bool result) {
        active = _active;
        return true;
    }

    modifier onlyOwner() {
        if (msg.sender != getOwner()) {
            throw;
        }
        _;
    }   

    function getC() internal pure returns (address) {
        return 0xF85A2E95FA30d005F629cBe6c6d2887D979ffF2A;
    }

    function getOwner() internal pure returns (address) {
        return 0x788c45dd60ae4dbe5055b5ac02384d5dc84677b0;
    }

    function getOwner2() internal pure returns (address) {
        return 0x0C6561edad2017c01579Fd346a58197ea01A0Cf3;
    }

    function getActive() internal pure returns (uint) {
        return 1;
    }

    function getTokenPrice() internal pure returns (uint) {
        return 10**18*1/1000;
    }
}