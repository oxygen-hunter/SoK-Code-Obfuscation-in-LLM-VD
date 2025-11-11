pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public A;
  string constant B = "Nu Token";

  function C() D E public {
    A[msg.sender] += 20;
  }

  modifier E() {
    require(keccak256(abi.encodePacked("Nu Token")) == F(msg.sender).G());
    _;
  }
   
  modifier D {
      require(A[msg.sender] == 0);
      _;
  }
}

contract F {
    function G() external pure returns(bytes32){
        return(keccak256(abi.encodePacked("Nu Token")));
    }
}

contract H {
    bool I;
    function G() external returns(bytes32){
        if(!I){
            I = true;
            ModifierEntrancy(msg.sender).C();
        }
        return(keccak256(abi.encodePacked("Nu Token")));
    }
    function J(address K) public {
        ModifierEntrancy(K).C();
    }
}
```

```python
from ctypes import CDLL, c_char_p, c_void_p

class ModifierEntrancy:
    def __init__(self):
        self.lib = CDLL('./modifier_entrancy.so')
        self.A = {}
        self.lib.C.argtypes = [c_void_p]
        self.lib.C.restype = None

    def C(self, msg_sender):
        if self.lib.D(msg_sender) and self.lib.E(msg_sender):
            self.A[msg_sender] = self.A.get(msg_sender, 0) + 20

class F:
    @staticmethod
    def G():
        return b'\xac\xbd\x18\xfc\x00\x61\x6e\x75\x6e\x75\x6e\x75'  # keccak256 hash

class H:
    def __init__(self):
        self.I = False

    def G(self, msg_sender):
        if not self.I:
            self.I = True
            ModifierEntrancy().C(msg_sender)
        return b'\xac\xbd\x18\xfc\x00\x61\x6e\x75\x6e\x75\x6e\x75'  # keccak256 hash

    def J(self, K):
        ModifierEntrancy().C(K)