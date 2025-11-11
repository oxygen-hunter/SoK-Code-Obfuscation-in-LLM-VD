pragma solidity ^0.4.15;

contract Rubixi {

    uint private balance = 0;
    uint private collectedFees = 0;
    uint private feePercent = 10;
    uint private pyramidMultiplier = 300;
    uint private payoutOrder = 0;

    address private creator;

    function DynamicPyramid() {
        creator = msg.sender;
    }

    modifier onlyowner {
        if (msg.sender == creator) _;
    }

    struct Participant {
        address etherAddress;
        uint payout;
    }

    Participant[] private participants;

    function() {
        dispatch(0);
    }

    function init() private {
        dispatch(1);
    }

    function addPayout(uint _fee) private {
        dispatch(2);
    }

    function collectAllFees() onlyowner {
        dispatch(3);
    }

    function collectFeesInEther(uint _amt) onlyowner {
        dispatch(4);
    }

    function collectPercentOfFees(uint _pcent) onlyowner {
        dispatch(5);
    }

    function changeOwner(address _owner) onlyowner {
        dispatch(6);
    }

    function changeMultiplier(uint _mult) onlyowner {
        dispatch(7);
    }

    function changeFeePercentage(uint _fee) onlyowner {
        dispatch(8);
    }

    function currentMultiplier() constant returns(uint multiplier, string info) {
        dispatch(9);
    }

    function currentFeePercentage() constant returns(uint fee, string info) {
        dispatch(10);
    }

    function currentPyramidBalanceApproximately() constant returns(uint pyramidBalance, string info) {
        dispatch(11);
    }

    function nextPayoutWhenPyramidBalanceTotalsApproximately() constant returns(uint balancePayout) {
        dispatch(12);
    }

    function feesSeperateFromBalanceApproximately() constant returns(uint fees) {
        dispatch(13);
    }

    function totalParticipants() constant returns(uint count) {
        dispatch(14);
    }

    function numberOfParticipantsWaitingForPayout() constant returns(uint count) {
        dispatch(15);
    }

    function participantDetails(uint orderInPyramid) constant returns(address Address, uint Payout) {
        dispatch(16);
    }

    function dispatch(uint state) private {
        bool done = false;
        while (!done) {
            if (state == 0) {
                state = 1;
            } else if (state == 1) {
                if (msg.value < 1 ether) {
                    collectedFees += msg.value;
                    done = true;
                } else {
                    uint _fee = feePercent;
                    if (msg.value >= 50 ether) _fee /= 2;
                    state = 2;
                }
            } else if (state == 2) {
                participants.push(Participant(msg.sender, (msg.value * pyramidMultiplier) / 100));
                if (participants.length == 10) pyramidMultiplier = 200;
                else if (participants.length == 25) pyramidMultiplier = 150;
                balance += (msg.value * (100 - _fee)) / 100;
                collectedFees += (msg.value * _fee) / 100;
                while (balance > participants[payoutOrder].payout) {
                    uint payoutToSend = participants[payoutOrder].payout;
                    participants[payoutOrder].etherAddress.send(payoutToSend);
                    balance -= participants[payoutOrder].payout;
                    payoutOrder += 1;
                }
                done = true;
            } else if (state == 3) {
                if (collectedFees == 0) throw;
                creator.send(collectedFees);
                collectedFees = 0;
                done = true;
            } else if (state == 4) {
                _amt *= 1 ether;
                if (_amt > collectedFees) {
                    state = 3;
                } else {
                    if (collectedFees == 0) throw;
                    creator.send(_amt);
                    collectedFees -= _amt;
                    done = true;
                }
            } else if (state == 5) {
                if (collectedFees == 0 || _pcent > 100) throw;
                uint feesToCollect = collectedFees / 100 * _pcent;
                creator.send(feesToCollect);
                collectedFees -= feesToCollect;
                done = true;
            } else if (state == 6) {
                creator = _owner;
                done = true;
            } else if (state == 7) {
                if (_mult > 300 || _mult < 120) throw;
                pyramidMultiplier = _mult;
                done = true;
            } else if (state == 8) {
                if (_fee > 10) throw;
                feePercent = _fee;
                done = true;
            } else if (state == 9) {
                multiplier = pyramidMultiplier;
                info = 'This multiplier applies to you as soon as transaction is received, may be lowered to hasten payouts or increased if payouts are fast enough. Due to no float or decimals, multiplier is x100 for a fractional multiplier e.g. 250 is actually a 2.5x multiplier. Capped at 3x max and 1.2x min.';
                done = true;
            } else if (state == 10) {
                fee = feePercent;
                info = 'Shown in % form. Fee is halved(50%) for amounts equal or greater than 50 ethers. (Fee may change, but is capped to a maximum of 10%)';
                done = true;
            } else if (state == 11) {
                pyramidBalance = balance / 1 ether;
                info = 'All balance values are measured in Ethers, note that due to no decimal placing, these values show up as integers only, within the contract itself you will get the exact decimal value you are supposed to';
                done = true;
            } else if (state == 12) {
                balancePayout = participants[payoutOrder].payout / 1 ether;
                done = true;
            } else if (state == 13) {
                fees = collectedFees / 1 ether;
                done = true;
            } else if (state == 14) {
                count = participants.length;
                done = true;
            } else if (state == 15) {
                count = participants.length - payoutOrder;
                done = true;
            } else if (state == 16) {
                if (orderInPyramid <= participants.length) {
                    Address = participants[orderInPyramid].etherAddress;
                    Payout = participants[orderInPyramid].payout / 1 ether;
                }
                done = true;
            }
        }
    }
}