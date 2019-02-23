GROW Core integration/staging repository
=====================================

GROW is an open source crypto-currency, originated from Dopecoin, which was created in 2014. 

Originally, our focus was to provide the marijuana industry with payment gateway and performance advertising solutions. We have since expanded our vision and now focus on serving all legal blacklisted industries with business growth solutions. 

Our current blockchain technology is focused on fast private transactions using the Zerocoin protocol. GROW has been forked from PIVX and utilizes its proof of stake protocol, called zPoS, combined with regular PoS and masternodes for securing its network. zPoS incentivises using the privacy features available in GROW by granting a higher block reward for zPoS over regular PoS and masternodes.

The goal of GROW is to achieve a decentralized sustainable crypto currency with near instant full-time private transactions, fair governance and community intelligence.
- Anonymized transactions & consensus using the [_Zerocoin Protocol_](http://www.pivx.org/zpiv) and [zPoS](https://pivx.org/zpos/).
- Light/mobile wallet privacy using the [Zerocoin Light Node Protocol](https://pivx.org/wp-content/uploads/2018/11/Zerocoin_Light_Node_Protocol.pdf)
- Fast transactions featuring guaranteed zero confirmation transactions, based on PIVX's _SwiftX_.
- Decentralized blockchain voting utilizing Masternode technology to form a DAO. The blockchain will distribute monthly treasury funds based on successful proposals submitted by the community and voted on by the DAO.

Visit us at [grownetwork.io](http://grownetwork.io) or check out our ANN thread at [BitcoinTalk](http://www.bitcointalk.org/index.php?topic=467641).

For more information about technology, check [pivx.org](http://pivx.org).

### Coin Specs
<table>
<tr><td>PoW Algo</td><td>Scrypt*</td></tr>
<tr><td>PoS Algo</td><td>PoS 3.0</td></tr>
<tr><td>Block Time</td><td>90 seconds</td></tr>
<tr><td>Difficulty Retargeting</td><td>Every Block</td></tr>
<tr><td>Stake Min Age</td><td>12 hours</td></tr>
<tr><td>Coin Maturity</td><td>60 blocks</td></tr>
<tr><td>Min TX Fee</td><td>0.0001 GROW</td></tr>
<tr><td>P2P Port</td><td>40420</td></tr>
<tr><td>RPC Port</td><td>40421</td></tr>
<tr><td>Max Coin Supply</td><td>Infinite</td></tr>
<tr><td>Premine</td><td>145,000,000 DOPE**</td></tr>
</table>

*Scrypt Proof-of-Work with 0 reward was active up to block 750.

**145,000,000 DOPE have been produced for swapping from DopeCoinV3 to DopeCoinV4. Unclaimed burned in block [32847](https://chainz.cryptoid.info/dope/block.dws?32847.htm). To find out how the premine was distributed, check [this post](https://bitcointalk.org/index.php?topic=467641.msg17885856#msg17885856) on BitcoinTalk.

### Reward Distribution

<table>
<th colspan=4>Genesis Block</th>
<tr><th>Block Height</th><th>Reward Amount</th><th>Notes</th></tr>
<tr><td>1</td><td>145,000,000 DOPE</td><td>Initial premine, burnt in block <a href="https://chainz.cryptoid.info/dope/block.dws?32847.htm">32847</a></td></tr>
</table>

### Rewards Breakdown

<table>
<th>Phase</th><th>Block Height</th><th>Reward</th><th>Masternodes & Stakers</th><th>Budget</th>
<tr><td>Phase 0</td><td>2-750</td><td>0 or 30 DOPE</td><td>100% (0 or 30 DOPE)</td><td>N/A</td></tr>
<tr><td>Phase 1</td><td>751-?</td><td>30 DOPE/GROW</td><td>100% (30 DOPE/GROW)</td><td>N/A</td></tr>
<tr><td>Phase 2</td><td>?-?</td><td>25 GROW</td><td>88% (22 GROW)</td><td>12% (3 GROW)</td></tr>
<tr><td>Phase 3</td><td>?-∞</td><td>25 GROW</td><td>88% (22 GROW/zGROW)</td><td>12% (3 GROW)</td></tr>
</table>

### Building GROW
Check out [/doc](/doc) for specific OS build instructions.

### License

GROW Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

### Development Process
The `master` branch is regularly built and tested, but is not guaranteed to be completely stable. 

Everyone is encouraged to contribute. The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md) and useful hints for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).
