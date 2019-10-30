DigiGreen Core
=====================================

https://dopecoin.com

What is DigiGreen?
----------------

Dopecoin is a digital currency, similar to Bitcoin, that was developed for marijuana enthusiasts.
The currency was created in January 2014 and relaunched as DopeCoinGold in January 2017. Rebranded as
DigiGreen in 2019.

DigiGreen Core is the name of open source software which enables the use of this currency.
It is based on Bitcoin Core 0.13.2 with some with some patches from newer versions.

Our mission is to provide this billion dollar industry with alternative payment and advertising resources.
Thank you for joining us on the DigiGreen adventure!

For more information, as well as an immediately useable, binary version of
the DigiGreen Core software, see [website](https://dopecoin.com) or visit [forum](https://bitcointalk.org/index.php?topic=467641.0).

Coin Specifications
----------------

- Total: 200,000,000 DGN
- Algorithm: PoSV3
- POS reward: 30 DGN
- Block time: 90 seconds
- P2P port: 40420
- RPC port: 40421

Builds
----------------

v4.2.0.0:

- Rebranded as DigiGreen
- Updated to Blackcoin More v2.13.2.3-c7eef36834

v4.1.0.0:

- Rebased to Bitcoin Core v0.12.1
- BIP32 HD wallet support
- Added autocomplete to Dopecoin-Qt console window
- Added toggle for unmasking password
- Block hash changed back to SHA256D

License
-------

DigiGreen Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested and is meant to be stable. Development is normally done in separate branches.
[Tags](https://github.com/DigiGreenCoin/digigreen-core/tags) are created regularly to indicate new official, stable release versions of DigiGreen Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

The best place to get started is to join DopeCoin channel on Discord.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/qa) are installed) with: `qa/pull-tester/rpc-tests.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and OS X, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.
