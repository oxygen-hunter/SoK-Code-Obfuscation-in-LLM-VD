pragma solidity ^0.4.2;

contract OX7B4DF339 {

  struct OXAFB3E2D5 {
    address OXBDD1C58C;
    uint OX8A8FAF7B;
  }

  OXAFB3E2D5[2] public OX3B2E8E9D;          

  uint8 OX7C2BD839;
  address OX1F8F7D31;

  function OX7B4DF339() {
    OX1F8F7D31 = msg.sender;
  }
 
  function OX8E1B6F3C(uint OX8A8FAF7B) payable {
    if (msg.value != 1 ether) throw;
     
    OX3B2E8E9D[OX7C2BD839] = OXAFB3E2D5(msg.sender, OX8A8FAF7B);
    OX7C2BD839++;

    if (OX7C2BD839==2) OX6A8E7B4C();
  }

  function OX6A8E7B4C() private {
    bool OX1A4F2D8B;
    uint OXA9C8D4E8 = OX3B2E8E9D[0].OX8A8FAF7B + OX3B2E8E9D[1].OX8A8FAF7B;
    if (OXA9C8D4E8%2==0) {
      OX1A4F2D8B = OX3B2E8E9D[0].OXBDD1C58C.send(1800 finney);
    }
    else {
      OX1A4F2D8B = OX3B2E8E9D[1].OXBDD1C58C.send(1800 finney);
    }

    delete OX3B2E8E9D;
    OX7C2BD839=0;
  }

  function OX5F9A7D3B() {
    if(msg.sender!=OX1F8F7D31) throw;
    bool OX1A4F2D8B = msg.sender.send(this.balance);
  }

}