# MiniC Medium Tests

## About

The CS325 coursework spec provides a set of simple tests for your MiniC compiler, and in previous years [students from the cohort starting in 2020 wrote a set of "harder tests"](https://github.com/codethulu/CS325-coursework-tests).

The harder test README succintly states it's goals as follows, which align with this repositories goals:

> Here are a few tests that will test your compiler's ability to handle aspects of the specification which are not tested by the default tests. If you cannot pass these tests, don't panic! As long as you pass the default tests I am sure you are capable of getting a high mark, these tests are for those who want to put their compilers through the wringer!

## Why is this helpful?

I found neither test set fully covered what I wanted testing:

* The simple tests don't exercise the compiler for bugs as fully as I would like.
* The harder tests have some cases I completely disagree with, and some which are technically valid but significantly extend the scope of the coursework.
* Neither test set provide negative testing for expected errors.

As a result of this, I decided to create my own test set to be used in combination with the existing two to fully exercise my compiler -- inspired by the excellent work done by previous students for the harder test set.

## What is added?

### Error message (negative) testing

The killer feature of this set of tests is negative testing!

This works by attempting to compile invalid code, then failing the test script if the compiler passes without error.

In addition to this, for invalid code which is also rejected by `clang`, the error message yielded by `clang` is also printed to model the output off.

### Specification semantics testing

Some tests for constraints of the specification beyond the largely syntactic simple tests are added, including:

* Testing scope, as inner and outer scopes have specific semantics for their interactions
* Testing truthiness, as integers and floats should be truthy as booleans in `if` and `while` statements
* Testing all permutations of widening casts, as this is easy to miss something if not rigorous
* Other miscellaneous tests, including:
  * Variable and function name shadowing
  * Nested trivial blocks
  * Left-associative operand correctness
  * `void` parameters
  * Expression statements


## Installation

As with the harder test set:

> Enter the directory where your coursework lives. Clone this repo, and you should be good to go. From there, just run the test script like you would the default tests.

## Recommended test methodology

1. Run (and pass) all the simple tests primarily
2. Install and try to pass all these medium tests
3. Install and try to pass some of the harder tests, consider excluding:
   * `implicit` - as this perform as a narrowing cast of floats to ints, so should not compiler.
   * `unary2` - this can be passed staying within the spec, but it's a right pain so I wouldn't bother.
   * `lazyeval` - this is only required if you are doing the extension of boolean short-circuiting.
