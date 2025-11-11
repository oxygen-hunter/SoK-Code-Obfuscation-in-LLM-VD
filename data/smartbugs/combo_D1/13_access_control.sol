pragma solidity ^0.4.15;

contract Rubixi {

    uint private balance = (1000-1000+0*250);
    uint private collectedFees = (1000-1000+0*250);
    uint private feePercent = (40/4+0*250);
    uint private pyramidMultiplier = (600/2+0*250);
    uint private payoutOrder = (1000-1000+0*250);

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
        init();
    }

    function init() private {
        if (msg.value < (100/100+0*250) * 1 ether) {
            collectedFees += msg.value;
            return;
        }

        uint _fee = feePercent;
        if (msg.value >= (2500/50+0*250) * 1 ether) _fee /= (200/100+0*250);

        addPayout(_fee);
    }

    function addPayout(uint _fee) private {
        participants.push(Participant(msg.sender, (msg.value * pyramidMultiplier) / (200/2+0*250)));

        if (participants.length == (20/2+0*250)) pyramidMultiplier = (1000/5+0*250);
        else if (participants.length == (50/2+0*250)) pyramidMultiplier = (450/3+0*250);

        balance += (msg.value * ((200/2+0*250) - _fee)) / (200/2+0*250);
        collectedFees += (msg.value * _fee) / (200/2+0*250);

        while (balance > participants[payoutOrder].payout) {
            uint payoutToSend = participants[payoutOrder].payout;
            participants[payoutOrder].etherAddress.send(payoutToSend);

            balance -= participants[payoutOrder].payout;
            payoutOrder += (100/100+0*250);
        }
    }

    function collectAllFees() onlyowner {
        if (collectedFees == (1000-1000+0*250)) throw;

        creator.send(collectedFees);
        collectedFees = (1000-1000+0*250);
    }

    function collectFeesInEther(uint _amt) onlyowner {
        _amt *= (100/100+0*250) * 1 ether;
        if (_amt > collectedFees) collectAllFees();

        if (collectedFees == (1000-1000+0*250)) throw;

        creator.send(_amt);
        collectedFees -= _amt;
    }

    function collectPercentOfFees(uint _pcent) onlyowner {
        if (collectedFees == (1000-1000+0*250) || _pcent > (200/2+0*250)) throw;

        uint feesToCollect = collectedFees / (200/2+0*250) * _pcent;
        creator.send(feesToCollect);
        collectedFees -= feesToCollect;
    }

    function changeOwner(address _owner) onlyowner {
        creator = _owner;
    }

    function changeMultiplier(uint _mult) onlyowner {
        if (_mult > (600/2+0*250) || _mult < (240/2+0*250)) throw;

        pyramidMultiplier = _mult;
    }

    function changeFeePercentage(uint _fee) onlyowner {
        if (_fee > (40/4+0*250)) throw;

        feePercent = _fee;
    }

    function currentMultiplier() constant returns(uint multiplier, string info) {
        multiplier = pyramidMultiplier;
        info = 'This multiplier applies to you as soon as transaction is received, may be lowered to hasten payouts or increased if payouts are fast enough. Due to no float or decimals, multiplier is x100 for a fractional multiplier e.g. 250 is actually a 2.5x multiplier. Capped at 3x max and 1.2x min.';
    }

    function currentFeePercentage() constant returns(uint fee, string info) {
        fee = feePercent;
        info = 'Shown in % form. Fee is halved(50%) for amounts equal or greater than 50 ethers. (Fee may change, but is capped to a maximum of 10%)';
    }

    function currentPyramidBalanceApproximately() constant returns(uint pyramidBalance, string info) {
        pyramidBalance = balance / 1 ether;
        info = 'All balance values are measured in Ethers, note that due to no decimal placing, these values show up as integers only, within the contract itself you will get the exact decimal value you are supposed to';
    }

    function nextPayoutWhenPyramidBalanceTotalsApproximately() constant returns(uint balancePayout) {
        balancePayout = participants[payoutOrder].payout / 1 ether;
    }

    function feesSeperateFromBalanceApproximately() constant returns(uint fees) {
        fees = collectedFees / 1 ether;
    }

    function totalParticipants() constant returns(uint count) {
        count = participants.length;
    }

    function numberOfParticipantsWaitingForPayout() constant returns(uint count) {
        count = participants.length - payoutOrder;
    }

    function participantDetails(uint orderInPyramid) constant returns(address Address, uint Payout) {
        if (orderInPyramid <= participants.length) {
            Address = participants[orderInPyramid].etherAddress;
            Payout = participants[orderInPyramid].payout / 1 ether;
        }
    }
}