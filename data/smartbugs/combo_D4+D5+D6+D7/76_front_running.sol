pragma solidity ^0.4.2;

contract OddsAndEvens{

  struct P {
    uint n;
    address a;
  }

  P[2] public ps;          

  uint8 t;
  address o;

  function OddsAndEvens() {
    o = msg.sender;
  }
 
  function play(uint n) payable{
    if (msg.value != 1 ether) throw;
     
    ps[t] = P(n, msg.sender);
    t++;

    if (t==2) a();
  }

  function a() private {
    bool r ;
    uint m = ps[0].n + ps[1].n;
    if (m%2==0) {
      r = ps[0].a.send(1800 finney);
    }
    else {
      r = ps[1].a.send(1800 finney);
    }

    delete ps;
    t=0;
  }

  function getProfit() {
    if(msg.sender!=o) throw;
    bool r = msg.sender.send(this.balance);
  }

}