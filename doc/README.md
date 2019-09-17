It is possible to generate the SNS-3 doxygen and sphinx documentation
without the need to have/compile the NS-3 source code.

You can see which kind of documentation is buildable using `make help`.

For instance, to generate the doxygen documentation, use: `make doxygen`.

Or to generate the sphinx documentation, use: `make html singlehtml`.

When using several targets, we strongly advise to use the `-k` option of
`make` to continue building remaining targets in case one of them fail. You
can also pass options to sphinx using the make variable `SPHINXOPTS`. _e.g._:

```
make SPHINXOPTS=-N -k doxygen html singlehtml latexpdf
```


All generated documentation will be stored in a new folder named `build`
