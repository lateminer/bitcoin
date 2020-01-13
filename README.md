Potcoin Core
=====================================

https://www.potcoin.com

What is Potcoin?
----------------

Potcoin is an experimental digital currency that enables instant payments to anyone, anywhere in the world. Potcoin was designed to empower, secure and facilitate the legal cannabis community’s transactions by creating a unique crypto currency for this thriving industry. Potcoin removes the need for cash transactions and encourages buying through consumer incentives.  On every level of the cannabis industry, users and supporters can entrust Potcoin to extend credibility, stability and security to this growing market.

The Potcoin mission is to economize the cannabis industry. Potcoin as a digital currency is an alternative payment network for cannabis users, merchants and industry professionals. The Potcoin network allows cannabis enthusiasts to interact, transact, communicate and grow together. Potcoin Core is the name of open source software which enables the use of this currency.

For more information, as well as an immediately useable, binary version of
the Potcoin Core software, see https://www.potcoin.com/wallets.

License
-------

Potcoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/Potcoin/potcoin-core/tags) are created
regularly to indicate new official, stable release versions of Potcoin Core.

Change log can be found in [CHANGELOG.md](CHANGELOG.md).

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

The best place to get started is to join Discord: https://discordapp.com/invite/KexXtC7

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
