Potcoin Core
=============

Setup
---------------------
Potcoin Core is the original Potcoin client and it builds the backbone of the network. However, it downloads and stores the entire history of Potcoin transactions (which is currently several GBs); depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more.

To download Potcoin Core, visit [potcoin.com](https://www.potcoin.com/wallets).

Running
---------------------
The following are some helpful notes on how to run Potcoin on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/potcoin-qt` (GUI) or
- `bin/potcoind` (headless)

### Windows

Unpack the files into a directory, and then run potcoin-qt.exe.

### OS X

Drag Potcoin-Core to your applications folder, and then run Potcoin-Core.

### Need Help?

* See the documentation at the [Bitcoin Wiki](https://en.bitcoin.it/wiki/Main_Page)
for help and more information.
* Ask for help in [Telegram channel](http://t.me/potcoin420).
* Ask for help in [Discord](https://discordapp.com/invite/KexXtC7).
* Ask for help on the [BitcoinTalk](https://bitcointalk.org/) forums, in the [Potcoin topic](https://bitcointalk.org/index.php?topic=426324.new#new).

Building
---------------------
The following are developer notes on how to build Blackcoin on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [OS X Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [Gitian Building Guide](gitian-building.md)

Development
---------------------
The Potcoin repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Multiwallet Qt Development](multiwallet-qt.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://dev.visucore.com/bitcoin/doxygen/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [Unit Tests](unit-tests.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)

### Resources
* Discuss on the [BitcoinTalk](https://bitcointalk.org/) forums, in the [Potcoin topic](https://bitcointalk.org/index.php?topic=426324.new#new).
* Discuss Potcoin development in [Telegram channel](http://t.me/potcoin420).
* Discuss Potcoin development in [Discord](https://discordapp.com/invite/KexXtC7).

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [Files](files.md)
- [Tor Support](tor.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)

License
---------------------
Distributed under the [MIT software license](http://www.opensource.org/licenses/mit-license.php).
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](https://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.
