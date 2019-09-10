Compile natively
================

.. note::
    As you certainly understand, writing a guide to install dependencies and
    compile such software on every type of linux distribution can be a tedious
    work and create lots of confusion. For this reason, this guide will use
    Debian as the Linux distribution of reference due to its universality and a
    high number of derivative works, for which these instructions would be
    somewhat applicable.


Prerequisites
-------------

* At least a dual-core processor.
* ~10 GB of persistent memory space.

This project is a derivative work of ns-3, so you are subject to their
`prerequisites and dependencies <https://www.nsnam.org/wiki/Installation>`_.
Please install them now to proceed.


Dependencies
------------

`rapidjson <http://rapidjson.org/>`_ version 1.10 or greater is needed to
compile and install IoD_Sim. Debian Stretch does not have a recent
``rapidjson-dev`` package, for which you should enable the ``backports``
repository.


Compile the source code
-----------------------

To prepare and compile the source code, please follow these steps below:

#. Clone ns-3 latest version from `git official repository
   <https://gitlab.com/nsnam/ns-3-dev>`_.
#. Copy or symlink ``drone`` directory in ``ns-3-dev/src/``.
#. Place yourself into ``ns-3-dev/`` directory.
#. Configure the source code using ``./waf configure``. More info are provided by ns-3 `documentation <https://www.nsnam.org/docs/release/3.29/tutorial/html/getting-started.html#building-with-waf>`_.
#. Build the source code with ``./waf build``.
