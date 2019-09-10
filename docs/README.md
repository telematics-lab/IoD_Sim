# The IoD_Sim Documentation

To build the documentation, please ensure that you have
[Sphinx](http://www.sphinx-doc.org/) installed. Sphinx requires `Python` 3.x,
and `pip` installed on your machine. If you are on a Unix-like environment,
ensure you also have `make` installed. Sphinx can be installed globally on your
development environment or using isolated environments such as
[virtualenv](https://virtualenv.pypa.io/en/latest/).

Once you have both `python` and `pip` available on your `PATH` environment
variable, you can install Sphinx and other dependencies by typing the following
command:

```
$  pip install -r requirements.txt
```

Finally, it is possible to build the documentation:

```
$  make html      #  Unix-like systems
>  make.bat html  // Windows
```

Please refer to [Sphinx documentation](http://www.sphinx-doc.org/) on building
this project on PDF or any other supported format.
