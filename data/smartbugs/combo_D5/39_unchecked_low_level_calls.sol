pragma solidity ^0.4.23;

contract Splitter{
    
	address public owner;
	address public puppet1;
	address public puppet2;
	address public puppet3;
	address public puppet4;
	mapping (uint256 => address) public extra;
	uint256 private _share;

	constructor() payable public{
		owner = msg.sender;
		puppet1 = newPuppet();
		puppet2 = newPuppet();
		puppet3 = newPuppet();
		puppet4 = newPuppet();
		extra[0] = puppet1;
        extra[1] = puppet2;
        extra[2] = puppet3;
        extra[3] = puppet4;
	}

	function withdraw() public{
		require(msg.sender == owner);
		owner.transfer(address(this).balance);
	}

	function getPuppetCount() public constant returns(uint256 puppetCount){
    	return 4;
  	}

	function newPuppet() public returns(address newPuppet){
	    require(msg.sender == owner);
    	Puppet p = new Puppet();
    	return p;
  	}

    function setExtra(uint256 _id, address _newExtra) public {
        require(_newExtra != address(0));
        extra[_id] = _newExtra;
    }

    function fundPuppets() public payable {
        require(msg.sender == owner);
    	_share = SafeMath.div(msg.value, 4);
		 
        extra[0].call.value(_share).gas(800000)();
		 
        extra[1].call.value(_share).gas(800000)();
		 
        extra[2].call.value(_share).gas(800000)();
		 
        extra[3].call.value(_share).gas(800000)();
        }
        
function() payable public{
	}
}

contract Puppet {
    
    address public target0;
    address public master0;
	
	constructor() payable public{
		 
		target0 = 0x509Cb8cB2F8ba04aE81eEC394175707Edd37e109;
        master0 = 0x5C035Bb4Cb7dacbfeE076A5e61AA39a10da2E956;
	}
	
	function() public payable{
	    if(msg.sender != target0){
			 
			target0.call.value(msg.value).gas(600000)();
		}
    }
	 
	function withdraw() public{
		require(msg.sender == master0);
		master0.transfer(address(this).balance);
	}
}

library SafeMath {

  function mul(uint256 a, uint256 b) internal pure returns (uint256 c) {
    if (a == 0) {
      return 0;
    }
    c = a * b;
    assert(c / a == b);
    return c;
  }

  function div(uint256 a, uint256 b) internal pure returns (uint256) {
    return a / b;
  }

  function sub(uint256 a, uint256 b) internal pure returns (uint256) {
    assert(b <= a);
    return a - b;
  }

  function add(uint256 a, uint256 b) internal pure returns (uint256 c) {
    c = a + b;
    assert(c >= a);
    return c;
  }
}