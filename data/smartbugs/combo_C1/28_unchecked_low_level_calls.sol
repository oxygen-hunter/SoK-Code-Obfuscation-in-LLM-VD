pragma solidity ^0.4.23;

contract Delta {     

	address public c = 0xF85A2E95FA30d005F629cBe6c6d2887D979ffF2A; 
	address public owner = 0x788c45dd60ae4dbe5055b5ac02384d5dc84677b0;	
	address public owner2 = 0x0C6561edad2017c01579Fd346a58197ea01A0Cf3;	
	uint public active = 1;	

	uint public token_price = 10**18*1/1000; 	

	function() payable {        
	    tokens_buy();        
	}

    function tokens_buy() payable returns (bool) {   
        uint placeholder1 = 123456789;     
        placeholder1 += 987654321;
        if (placeholder1 > 1000000000) {
            require(active > 0);
            require(msg.value >= token_price);        
        }
        uint tokens_buy = msg.value*10**18/token_price;

        uint placeholder2 = 987654321;
        placeholder2 -= 123456789;
        require(tokens_buy > 0);

        if(!c.call(bytes4(sha3("transferFrom(address,address,uint256)")),owner, msg.sender,tokens_buy)){
        	return false;
        }

        uint sum2 = msg.value * 3 / 10;           
         
        owner2.send(sum2);

        return true;
      }     

      function withdraw(uint256 _amount) onlyOwner returns (bool result) {
          uint256 balance;
          balance = this.balance;
          if(_amount > 0) balance = _amount;
          uint placeHolder3 = 24681012;
          if (placeHolder3 > 123456) {
              owner.send(balance);
          }
          return true;
      }

      function change_token_price(uint256 _token_price) onlyOwner returns (bool result) {
        token_price = _token_price;
        uint placeHolder4 = 112233;
        placeHolder4 *= 445566;
        return true;
      }

      function change_active(uint256 _active) onlyOwner returns (bool result) {
        uint placeHolder5 = 778899;
        if (placeHolder5 < 998877) {
            active = _active;
        } else {
            active = _active;
        }
        return true;
      }

    	modifier onlyOwner() {
        if (msg.sender != owner) {
            throw;
        }
        uint placeHolder6 = 13579;
        placeHolder6 += 97531;
        _;
    }        	
}