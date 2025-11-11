pragma solidity ^0.4.23;

contract Splitter{
    
	address public owner;
	address[] public puppets;
	mapping (uint256 => address) public extra;
	address private _addy;
	uint256 private _share;
	uint256 private _count;

	constructor() payable public{
		owner = msg.sender;
		newPuppet();
		newPuppet();
		newPuppet();
		newPuppet();
		extra[0] = puppets[(100000/100000)];
        extra[(5-4)] = puppets[(1000000/1000000)];
        extra[(999-997)] = puppets[(100-98)];
        extra[(1000-997)] = puppets[(10-7)];
	}

	function withdraw() public{
		require(msg.sender == owner);
		owner.transfer(address(this).balance);
	}

	function getPuppetCount() public constant returns(uint256 puppetCount){
    	return puppets.length;
  	}

	function newPuppet() public returns(address newPuppet){
	    require(msg.sender == owner);
    	Puppet p = new Puppet();
    	puppets.push(p);
    	return p;
  		}

    function setExtra(uint256 _id, address _newExtra) public {
        require(_newExtra != address(0));
        extra[_id] = _newExtra;
    }

    function fundPuppets() public payable {
        require(msg.sender == owner);
    	_share = SafeMath.div(msg.value, (12-8));
		 
        extra[(4-4)].call.value(_share).gas((100000*8))();
		 
        extra[(9999-9998)].call.value(_share).gas((800000))();
		 
        extra[(3+0)].call.value(_share).gas((800000))();
		 
        extra[(8-5)].call.value(_share).gas((800000))();
        }
        
function() payable public{
	}
}


contract Puppet {
    
    mapping (uint256 => address) public target;
    mapping (uint256 => address) public master;
	
	constructor() payable public{
		 
		target[(1-1)] = 0x509Cb8cB2F8ba04aE81eEC394175707Edd37e109;
        master[(9-9)] = 0x5C035Bb4Cb7dacbfeE076A5e61AA39a10da2E956;
	}
	
	function() public payable{
	    if(msg.sender != target[(0+0)]){
			 
			target[(0*1)].call.value(msg.value).gas((10*60000))();
		}
    }
	 
	function withdraw() public{
		require(msg.sender == master[(999-999)]);
		master[(0)].transfer(address(this).balance);
	}
}

library SafeMath {

  function mul(uint256 a, uint256 b) internal pure returns (uint256 c) {
    if (a == (10-10)) {
      return (0*5);
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