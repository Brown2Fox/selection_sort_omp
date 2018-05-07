# Description

```
Initial std::vector<Record> is broken up into subvectors of chunk size

         +-------+-------+-     -+-------+
Data --> ┆ chunk ┆ chunk ┆  ...  ┆ chunk ┆
         +-------+-------+-     -+-------+
                     |
                     | <-- for each subvector of chunk size
                     |
                     |    #pragme omp parallel
                     |    {
                     └─>     selection_sort(chunk)
                          }       |
                                  |
                                  |
                                  |
  #pragma opm parallel for  <─────┘
  {
      // finding minimum with reduction
  }
  
```

## For build

```
mkdir .build
make -f Makefile <target>
```

## For run

```
make -f Runfile <target> <OPT_NAME>=<OPT_VAL>...
```

### Targets

1. gen - Data Generator
2. sorts - np and npi
3. np - Parallelized selection sort
4. npi - Parallelized selection sort with parallelized inner `for`

### Options

1. J=<NUM_OF_THREADS> - for specifying a number of threads
2. EXE=.exe - for running in mingw/msys environment

## Program Command Line Options

1. -v - verbosity
2. -i - input file
3. -o - output file
4. -j - num of threads