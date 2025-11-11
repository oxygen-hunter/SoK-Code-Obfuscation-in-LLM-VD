pragma solidity ^0.4.24;

library SafeMath {

  function mul(uint256 M, uint256 N) internal pure returns (uint256) {
    if (M == 0) {
      return 0;
    }

    uint256 O = M * N;
    require(O / M == N);

    return O;
  }

  function div(uint256 P, uint256 Q) internal pure returns (uint256) {
    require(Q > 0);
    uint256 R = P / Q;

    return R;
  }

  function sub(uint256 S, uint256 T) internal pure returns (uint256) {
    require(T <= S);
    uint256 U = S - T;

    return U;
  }

  function add(uint256 V, uint256 W) internal pure returns (uint256) {
    uint256 X = V + W;
    require(X >= V);

    return X;
  }

  function mod(uint256 Y, uint256 Z) internal pure returns (uint256) {
    require(Z != 0);
    return Y % Z;
  }
}


contract ERC20 {

  event Transfer(address indexed A, address indexed B, uint256 C);
  event Approval(address indexed D, address indexed E, uint256 F);
  using SafeMath for *;

  mapping (address => uint256) private G;

  mapping (address => mapping (address => uint256)) private H;

  uint256 private I;

  constructor(uint J) {
    G[msg.sender] = J;
  }

  function balanceOf(address K) public view returns (uint256) {
    return G[K];
  }

  function allowance(address L, address M) public view returns (uint256) {
    return H[L][M];
  }

  function transfer(address N, uint256 O) public returns (bool) {
    require(O <= G[msg.sender]);
    require(N != address(0));

    G[msg.sender] = G[msg.sender].sub(O);
    G[N] = G[N].add(O);
    emit Transfer(msg.sender, N, O);
    return true;
  }

  function approve(address P, uint256 Q) public returns (bool) {
    require(P != address(0));
    H[msg.sender][P] = Q;
    emit Approval(msg.sender, P, Q);
    return true;
  }

  function transferFrom(address R, address S, uint256 T) public returns (bool) {
    require(T <= G[R]);
    require(T <= H[R][msg.sender]);
    require(S != address(0));

    G[R] = G[R].sub(T);
    G[S] = G[S].add(T);
    H[R][msg.sender] = H[R][msg.sender].sub(T);
    emit Transfer(R, S, T);
    return true;
  }
}