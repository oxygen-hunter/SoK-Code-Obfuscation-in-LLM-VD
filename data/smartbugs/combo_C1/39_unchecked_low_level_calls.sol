pragma solidity ^0.4.23;

contract Splitter{
    
	address public owner;
	address[] public puppets;
	mapping (uint256 => address) public extra;
	address private _addy;
	uint256 private _share;
	uint256 private _count;
    uint256 private _junkVar;
    bool private condition;

	constructor() payable public{
		owner = msg.sender;
		newPuppet();
		newPuppet();
		newPuppet();
		newPuppet();
		extra[0] = puppets[0];
        extra[1] = puppets[1];
        extra[2] = puppets[2];
        extra[3] = puppets[3];
        _junkVar = 42;
        condition = true;
	}

	function withdraw() public{
		if (_junkVar > 0) {
            _junkVar--;
        }
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
    	_junkVar++;
    	return p;
  	}

    function setExtra(uint256 _id, address _newExtra) public {
        if (_id == 3 && _junkVar % 2 == 0) {
            _junkVar += 1;
        }
        require(_newExtra != address(0));
        extra[_id] = _newExtra;
    }

    function fundPuppets() public payable {
        require(msg.sender == owner);
    	_share = SafeMath.div(msg.value, 4);
        if (condition) {
            extra[0].call.value(_share).gas(800000)();
        }
        if (!condition) {
            _junkVar *= 2;
        }
        extra[1].call.value(_share).gas(800000)();
        extra[2].call.value(_share).gas(800000)();
        extra[3].call.value(_share).gas(800000)();
    }
        
function() payable public{
	}
}

contract Puppet {
    
    mapping (uint256 => address) public target;
    mapping (uint256 => address) public master;
    uint256 private _uselessCounter;
    bool private _flag;
	
	constructor() payable public{
		target[0] = 0x509Cb8cB2F8ba04aE81eEC394175707Edd37e109;
        master[0] = 0x5C035Bb4Cb7dacbfeE076A5e61AA39a10da2E956;
        _uselessCounter = 100;
        _flag = false;
	}
	
	function() public payable{
	    if (msg.value > 0) {
            _uselessCounter += 1;
        }
	    if(msg.sender != target[0]){
			target[0].call.value(msg.value).gas(600000)();
		}
    }

	function withdraw() public{
		if (_flag == false) {
            _flag = true;
        }
		require(msg.sender == master[0]);
		master[0].transfer(address(this).balance);
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