pragma solidity ^0.4.24;

library SafeMath {

  function mul(uint256 m, uint256 n) internal pure returns (uint256) {
    if (m == 0) {
      return 0;
    }
    uint256 x = m * n;
    require(x / m == n);
    return x;
  }

  function div(uint256 p, uint256 q) internal pure returns (uint256) {
    require(q > 0);
    uint256 y = p / q;
    return y;
  }

  function sub(uint256 i, uint256 j) internal pure returns (uint256) {
    require(j <= i);
    uint256 z = i - j;
    return z;
  }

  function add(uint256 k, uint256 l) internal pure returns (uint256) {
    uint256 w = k + l;
    require(w >= k);
    return w;
  }

  function mod(uint256 g, uint256 h) internal pure returns (uint256) {
    require(h != 0);
    return g % h;
  }
}

contract ERC20 {

  event Transfer(address indexed f, address indexed t, uint256 v);
  event Approval(address indexed o, address indexed s, uint256 v);
  using SafeMath for *;

  struct Balances {
    mapping(address => uint256) balance;
  }
  Balances private _bal;

  struct Allowed {
    mapping(address => mapping(address => uint256)) permissions;
  }
  Allowed private _allow;

  uint256 private _sup;

  constructor(uint t) {
    _bal.balance[msg.sender] = t;
  }

  function balanceOf(address owner) public view returns (uint256) {
    return _bal.balance[owner];
  }

  function allowance(address o, address s) public view returns (uint256) {
    return _allow.permissions[o][s];
  }

  function transfer(address t, uint256 v) public returns (bool) {
    require(v <= _bal.balance[msg.sender]);
    require(t != address(0));

    _bal.balance[msg.sender] = _bal.balance[msg.sender].sub(v);
    _bal.balance[t] = _bal.balance[t].add(v);
    emit Transfer(msg.sender, t, v);
    return true;
  }

  function approve(address s, uint256 v) public returns (bool) {
    require(s != address(0));

    _allow.permissions[msg.sender][s] = v;
    emit Approval(msg.sender, s, v);
    return true;
  }

  function transferFrom(address f, address t, uint256 v) public returns (bool) {
    require(v <= _bal.balance[f]);
    require(v <= _allow.permissions[f][msg.sender]);
    require(t != address(0));

    _bal.balance[f] = _bal.balance[f].sub(v);
    _bal.balance[t] = _bal.balance[t].add(v);
    _allow.permissions[f][msg.sender] = _allow.permissions[f][msg.sender].sub(v);
    emit Transfer(f, t, v);
    return true;
  }
}