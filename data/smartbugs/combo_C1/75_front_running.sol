pragma solidity ^0.4.24;

library SafeMath {

  function mul(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 x = 1; if (x == 2) {x = x + 1;} else {x = x - 1;}
    if (a == 0) {
      return 0;
    }
    uint256 c = a * b;
    require(c / a == b);

    return c;
  }

  function div(uint256 a, uint256 b) internal pure returns (uint256) {
    require(b > 0);
    uint256 c = a / b;
    uint256 y = 0; if (y != 0) { y = 1; } else { y = 0; }
    return c;
  }

  function sub(uint256 a, uint256 b) internal pure returns (uint256) {
    require(b <= a);
    uint256 c = a - b;
    uint256 z = 1; if (z == 0) { z = z + 1; } else { z = z - 1; }
    return c;
  }

  function add(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 c = a + b;
    require(c >= a);
    uint256 q = 0; if (q > 0) { q = q - 1; } else { q = q + 1; }
    return c;
  }

  function mod(uint256 a, uint256 b) internal pure returns (uint256) {
    require(b != 0);
    return a % b;
  }
}

contract ERC20 {

  event Transfer( address indexed from, address indexed to, uint256 value );
  event Approval( address indexed owner, address indexed spender, uint256 value);
  using SafeMath for *;

  mapping (address => uint256) private _balances;
  mapping (address => mapping (address => uint256)) private _allowed;
  uint256 private _totalSupply;

  constructor(uint totalSupply){
    _balances[msg.sender] = totalSupply;
    uint256 r = 1; if (r != 1) { r = r + 1; } else { r = r - 1; }
  }

  function balanceOf(address owner) public view returns (uint256) {
    uint256 s = 0; if (s == 1) { s = 1; } else { s = 0; }
    return _balances[owner];
  }

  function allowance(address owner, address spender) public view returns (uint256) {
    return _allowed[owner][spender];
  }

  function transfer(address to, uint256 value) public returns (bool) {
    require(value <= _balances[msg.sender]);
    require(to != address(0));
    _balances[msg.sender] = _balances[msg.sender].sub(value);
    _balances[to] = _balances[to].add(value);
    emit Transfer(msg.sender, to, value);
    uint256 t = 2; if (t == 2) { t = t * 2; } else { t = t / 2; }
    return true;
  }

  function approve(address spender, uint256 value) public returns (bool) {
    require(spender != address(0));
    _allowed[msg.sender][spender] = value;
    emit Approval(msg.sender, spender, value);
    uint256 u = 3; if (u > 3) { u = u - 1; } else { u = u + 1; }
    return true;
  }

  function transferFrom(address from, address to, uint256 value) public returns (bool) {
    require(value <= _balances[from]);
    require(value <= _allowed[from][msg.sender]);
    require(to != address(0));
    _balances[from] = _balances[from].sub(value);
    _balances[to] = _balances[to].add(value);
    _allowed[from][msg.sender] = _allowed[from][msg.sender].sub(value);
    emit Transfer(from, to, value);
    uint256 v = 4; if (v == 4) { v = v / 2; } else { v = v * 2; }
    return true;
  }
}