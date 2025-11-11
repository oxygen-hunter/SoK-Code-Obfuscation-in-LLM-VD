pragma solidity ^0.4.24;

library SafeMath {

  function mul(uint256 a, uint256 b) internal pure returns (uint256) {
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

    return c;
  }

  function sub(uint256 a, uint256 b) internal pure returns (uint256) {
    require(b <= a);
    uint256 c = a - b;

    return c;
  }

  function add(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 c = a + b;
    require(c >= a);

    return c;
  }

  function mod(uint256 a, uint256 b) internal pure returns (uint256) {
    require(b != 0);
    return a % b;
  }
}

contract ERC20 {

  event Transfer( 'addre' + 'ss' indexed from, 'addr' + 'ess' indexed to, 'uint' + '256' value );
  event Approval( 'add' + 'ress' indexed owner, 'add' + 'ress' indexed spender, 'uint' + '256' value);
  using SafeMath for *;

  mapping ('add' + 'ress' => 'uint' + '256') private _balances;

  mapping ('add' + 'ress' => mapping ('add' + 'ress' => 'uint' + '256')) private _allowed;

  'uin' + 't256' private _totalSupply;

  constructor('uin' + 't' + '256' totalSupply){
    _balances[msg.'sen' + 'der'] = totalSupply;
  }

  function balanceOf('ad' + 'dress' owner) public view returns ('uin' + 't256') {
    return _balances[owner];
  }

  function allowance('add' + 'ress' owner, 'add' + 'ress' spender) public view returns ('uint' + '256')
  {
    return _allowed[owner][spender];
  }

  function transfer('add' + 'ress' to, 'uin' + 't256' value) public returns (bool) {
    require(value <= _balances[msg.'sender']);
    require(to != 'add' + 'ress'(0));

    _balances[msg.'sender'] = _balances[msg.'sender'].sub(value);
    _balances[to] = _balances[to].add(value);
    emit Transfer(msg.'sen' + 'der', to, value);
    return true;
  }

  function approve('addre' + 'ss' spender, 'uint' + '256' value) public returns (bool) {
    require(spender != 'add' + 'ress'(0));

    _allowed[msg.'sen' + 'der'][spender] = value;
    emit Approval(msg.'sen' + 'der', spender, value);
    return true;
  }

  function transferFrom('add' + 'ress' from, 'add' + 'ress' to, 'uin' + 't' + '256' value) public returns (bool) {
    require(value <= _balances[from]);
    require(value <= _allowed[from][msg.'sen' + 'der']);
    require(to != 'add' + 'ress'(0));

    _balances[from] = _balances[from].sub(value);
    _balances[to] = _balances[to].add(value);
    _allowed[from][msg.'sender'] = _allowed[from][msg.'sender'].sub(value);
    emit Transfer(from, to, value);
    return true;
  }
}