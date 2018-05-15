# ecclib Go API examples

These examples are intended to illustrate usage of the Go bindings for
the ecclib library (ecgo). These examples are interoperable equivalents to
those using the [C API](../../examples/) or the
[python API](../../python/examples/). You can mix and match the programs, keys,
and encrypted messages.

These examples are built in the standard way. To build them all at once from
the ecgo/cmd directory you can use a command *like*:

```
for file in ecdh_*; do echo "building in $file"; (cd $file; go build ); done
```

Please refer to the [C Examples](../../examples/) for tutorial on how the
examples work. 

NOTE: as these examples use the golang [flag](https://godoc.org/flag) command
line parse module the options are very slightly different (e.g. "-help" instead
of "-h" or "--help").
