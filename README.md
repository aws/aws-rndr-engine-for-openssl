# AWS RNDR Engine for OpenSSL
A Random Number Generation Engine for OpenSSL making use of the Arm instruction
RNDR.

## Build Requirements
* CMake
* OpenSSL Development files (ie openssl-devel/libssl-dev)
  * The development files' versions must match the version of OpenSSL which
    will run the engine.
* C compiler
* OpenSSL
* Perl
* Target Host of Arm 64 CPU

## Test and Run Requirements
* OpenSSL
* Host running on Arm 64 CPU which has access to RNDR and RNDRRS instructions

## Installation
Run once:
```
mkdir build
cd build
cmake ../
```

### Quick Install
```
make
make install
```

### Configuring Build
```
cmake ../
```
Run `cmake --help` for details regarding configuration options.

Some useful configuration options are:
```
  -DCMAKE_INSTALL_PREFIX=DIR    install library to specified directory prefix
  -DCMAKE_INSTALL_LIBDIR=DIR    install library to specified directory
  -DOPENSSL_ROOT_DIR=DIR        set destination for OpenSSL root directory
  -DCMAKE_C_FLAGS=FLAGS         set additional CFLAGS for compilation
```

<details>
    <summary>Installing to a non-default engine location</summary>

Engine libraries (`eng_rndr.so`) are installed by default to
`${CMAKE_INSTALL_LIBDIR}` where `${CMAKE_INSTALL_LIBDIR}` usually refers to
`/usr/local/lib/`. This location can be overwritten in the
configurations using `-DCMAKE_INSTALL_PREFIX=DIR`.

i.e. To install the engine library to `/usr/lib/aarch64-linux-gnu/engines-1.1/`
```
cmake -DCMAKE_INSTALL_PREFIX=/usr/lib/aarch64-linux-gnu/engines-1.1/ ../
```
</details>
### Make
```
make
```
Generated shared library files `libeng_rndr.so*` will be located in `./build`.

### Testing
Verify that random number generation functions for the engine work.
```
make test ARGS="-V"
```
The output will generate test run messages.
```
Loading test...
Running 'sanity_check_rndr_bytes'...
Test succeeded
Running 'sanity_check_rndrrs_bytes'...
Test succeeded
```
Test the engine built successfully and can be installed
```
openssl engine -t -c src/.libs/libeng_rndr.so
```
This will generate the engines details and availability.
```
(eng_rndr) Arm RNDR engine
Loaded: (rndr) Arm RNDR engine
 [RAND]
     [ available ]
```
Test random number generating using the engine.
```
openssl rand -engine build/libeng_rndr.so -hex 10
```
This will display the randomly generated 10 hex numbers.
```
engine "rndr" set.
01c1269d93d9f01ebff4
```

### Installation
Installation may require root privileges. To install, run:
```
make install
```
<details>
    <summary>Environment Variable</summary>

Set `export OPENSSL_ENGINES=INSTALLATION_DIR` environment variable in shell
startup files. This will allow openssl to find the RNDR engine.

If using OpenSSL 1.0.2, the engine will be called `eng_rndr`. If using OpenSSL
1.1.1 or above the engine will be called `libeng_rndr`.

Verify installation works
`openssl engine -t -c libeng_rndr`
```
...
(rndr) Arm RNDR engine
 [RAND]
     [ available ]
```
</details>

<details>
    <summary>Dynamic Engine Installation</summary>

After installing update `openssl.cnf` to contain the following.

```
openssl_conf = openssl_def

[openssl_def]
engines = engine_section

[engine_section]
eng_rndr = eng_rndr_section

[eng_rndr_section]
engine_id = libeng_rndr
dynamic_path = <PATH TO INSTALLED libeng_rndr.so>
init = 0
```

Verify installation works
`openssl engine -t -c`
```
...
(rndr) Arm RNDR engine
 [RAND]
     [ available ]
```
</details>

## Security

See [CONTRIBUTING](CONTRIBUTING.md#security-issue-notifications) for more information.

## License

This project is licensed under the Apache-2.0 License.
