FLAGS=-Wall -std=c++17 -O3 -march=skylake -DNDEBUG

run_nb: benchmark_nb
	./benchmark_nb 100000

run_nb_gdb: benchmark_nb
	gdb ./benchmark_nb

%.in.o:
	: Do nothing

benchmark: benchmark.cpp verb-parse-*.cpp *.h *.hpp compile_flags.txt
	$(CXX) $(FLAGS) benchmark.cpp http_verb.cpp -o $@

benchmark_nb: benchmark_nb.cpp nanobench.o verb-parse-*.cpp *.h *.hpp compile_flags.txt
	$(CXX) $(FLAGS) benchmark_nb.cpp nanobench.o http_verb.cpp -o $@

swar32-cache-misses.perf.data: benchmark_nb
	env NANOBENCH_ENDLESS=swar32 timeout 3 perf record -o $@ -e branch-misses taskset -c 1 ./benchmark_nb

verb-parse-perfhash.in.cpp: http_verbs.txt
	gperf --language=C++\
          --enum\
          --readonly-tables\
          --compare-strncmp\
          --class-name=perfect_hash\
          --lookup-function-name=string_to_verb\
          --output-file=$@\
          http_verbs.txt
	sed -i 's/register//g' $@

compile_flags.txt: Makefile
	echo $(FLAGS) | sed 's/\s\+/\n/g' > compile_flags.txt
