### HTTP Verb parsing improvements

This repo is a fork of the HTTP verb [parsing benchmarks](http://0x80.pl/notesen/2022-01-29-http-verb-parse.html) by Wojciech Muła.

The benchmarks use the [nanobench](https://github.com/martinus/nanobench) benchmarking framework.

#### Running the benchmarks

To build and run the benchmarks, just run `make run_nb`.

To ensure consistent benchmarks, it is best to disable turboboost and fix a frequency.
You can do so on Linux by running `python benchmark_mode.py benchmarking`.

Also, it is best to pin the benchmark code to a single core. This can be done by using the `taskset` util on Linux.

