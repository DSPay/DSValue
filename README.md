DSValue integration/staging tree
=====================================

https://www.dspay.org

Copyright (c) 2009-2014 Bitcoin Developers
Copyright (c) 2014-2014 DSPay Developers

What is DSValue?
----------------

DSValue  is  a  creative  cryptocurrency  based  on  SatoshiNakamoto’s Bitcoin. 
It embraces the P2P lottery that 95% lottery's take goes back out as prizes. 
The random numbers  are  generated  from the block, and the prize will be 
distributed  automatically .  All  the  data  are  protected  by cryptography 
theory ,  which insures no any individual or a third party can tamper with 
it (except for the 51% attack).

For more information, as well as an immediately useable, binary version of
the Bitcoin Core software, see https://www.dspay.org/download.

License
-------

DSValue is released under the terms of the MIT license. See `COPYING` for more
information or see http://opensource.org/licenses/MIT.

Development process
-------------------

Developers work in their own trees, then submit pull requests when they think
their feature or bug fix is ready.

If it is a simple/trivial/non-controversial change, then one of the DSPay
development team members simply pulls it.

If it is a *more complicated or potentially controversial* change, then the patch
submitter will be asked to start a discussion (if they haven't already) on the
[mailing list](http://sourceforge.net/mailarchive/forum.php?forum_name=bitcoin-development).

The patch will be accepted if there is broad consensus that it is a good thing.
Developers should expect to rework and resubmit patches if the code doesn't
match the project's coding conventions (see [doc/coding.md](doc/coding.md)) or are
controversial.

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/bitcoin/bitcoin/tags) are created
regularly to indicate new official, stable release versions of DSValue.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write unit tests for new code, and to
submit new unit tests for old code.

Unit tests for the core code are in `src/test/`.
