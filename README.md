# Chaos

<p align="center">
  <img src="https://chaos-lang.org/img/chaos_logo.png" alt="Logo" height="300px"/>
</p>

<p align="center">
    Turn chaos into magic!
</p>
<p align="center">
    <img alt="GitHub Last Commit" src="https://img.shields.io/github/last-commit/chaos-lang/chaos?logo=GitHub&style=flat-square">
    <img alt="GitHub Commit Activity" src="https://img.shields.io/github/commit-activity/m/chaos-lang/chaos?logo=GitHub&style=flat-square">
    <a href="https://github.com/chaos-lang/chaos/blob/master/LICENSE">
        <img alt="GitHub License" src="https://img.shields.io/github/license/chaos-lang/chaos?logo=GitHub&style=flat-square">
    </a>
    <a href="https://github.com/chaos-lang/chaos/actions?query=workflow%3A%22Linter+Checks%22">
        <img alt="GitHub Workflow Linter Checks Status" src="https://img.shields.io/github/workflow/status/chaos-lang/chaos/Linter%20Checks?logo=GitHub&label=linter%20checks&style=flat-square">
    </a>
    <a href="https://github.com/chaos-lang/chaos/actions?query=workflow%3A%22Automated+Tests%22">
        <img alt="GitHub Workflow Automated Tests Status" src="https://img.shields.io/github/workflow/status/chaos-lang/chaos/Automated%20Tests?logo=GitHub&label=automated%20tests&style=flat-square">
    </a>
    <a href="https://github.com/chaos-lang/chaos/actions?query=workflow%3A%22Memory+Leak+Checks%22">
        <img alt="GitHub Workflow Memory Leak Checks" src="https://img.shields.io/github/workflow/status/chaos-lang/chaos/Memory%20Leak%20Checks?logo=GitHub&label=memcheck&style=flat-square">
    </a>
    <a href="https://github.com/chaos-lang/chaos/actions?query=workflow%3A%22Memory+Sanitizer%22">
        <img alt="GitHub Workflow Memory Sanitizer" src="https://img.shields.io/github/workflow/status/chaos-lang/chaos/Memory%20Sanitizer?logo=GitHub&label=memory%20sanitizer&style=flat-square">
    </a>
    <a href="https://github.com/chaos-lang/chaos/actions?query=workflow%3A%22Address+Sanitizer%22">
        <img alt="GitHub Workflow Address Sanitizer" src="https://img.shields.io/github/workflow/status/chaos-lang/chaos/Address%20Sanitizer?logo=GitHub&label=address%20sanitizer&style=flat-square">
    </a>
    <a href="https://github.com/chaos-lang/chaos/actions?query=workflow%3A%22Undefined+Behavior+Sanitizer%22">
        <img alt="GitHub Workflow Undefined Behavior Sanitizer" src="https://img.shields.io/github/workflow/status/chaos-lang/chaos/Undefined%20Behavior%20Sanitizer?logo=GitHub&label=undefined%20behavior%20sanitizer&style=flat-square">
    </a>
    <a href="https://github.com/chaos-lang/chaos/actions?query=workflow%3A%22Chaos+C+Extension+Feature+Tests%22">
        <img alt="GitHub Workflow Chaos C Extension Feature Tests Status" src="https://img.shields.io/github/workflow/status/chaos-lang/chaos/Chaos%20C%20Extension%20Feature%20Tests?logo=GitHub&label=Chaos%20C%20Extension%20Feature&style=flat-square">
    </a>
</p>
<p align="center">
    Chaos is a purely functional programming language that achieves zero cyclomatic complexity.
</p>

### Influenced by

 - TypeScript's type safety
 - Python's syntax, modules and extensibility
 - JavaScript's availability
 - Ruby's loops and blocks
 - PHP's dedication to server-side
 - Haskell's immutability
 - C's speed
 - NumPy's matrix arithmetics
 - Perl's regex engine

### Unique Features

 - There are no control structures. (no `if..else`, no `switch..case`) Decision making only possible on function returns:
   ```
   func myFunc(num a, num b)
        // your code
   end { x > 0 : f1(x), default : f2(x) }
   ```
 - A single unit test is enough to have 100% coverage on functions, always.
 - Warns you about untested functions in the executed program.
 - Chaos language is not object-oriented. So everything is done by functions and data types.
 - Immutability everywhere.
 - No `while` or `when` keywords. Loops are predetermined. There is also `INFINITE` keyword to achieve infinite loops if it's necessary.
 - Single source of errors and unique exit codes for each one of them. So the errors are catchable by other programs.
 - Language's itself forces you to write less error-prone code.
 - Say goodbye to dependency conflicts with [Occultist](https://occultist.io/) dependency manager.
 - Eliminated segmentation fault possibility using strict memory management and predictions.
 - Simplest [C extension development](https://chaos-lang.org/docs/16_chaos_c_extensions_development) framework so far.

## Installation

Install the requirements:

```
make requirements
```

Compile the interpreter:

```
make
```

Install the `chaos` binary system-wide:

```
make install
```

## Usage

### Interactive Shell

```
$ chaos
    Chaos Language 0.0.1-alpha (Apr 10 2020 01:22:17)
    GCC version: 7.4.0 on linux
    Turn chaos into magic!

kaos> print "hello world"
hello world
kaos>
```

### Program File as Command-line Argument

```
$ chaos dev.kaos
hello world
```

## Uninstallation

You can uninstall the `chaos` binary and its C headers with:

```
make uninstall
```

## Useful Links

[**Language Reference**](https://chaos-lang.org/docs/04_primitive-data-types)

[**Developing Chaos C Extensions**](https://chaos-lang.org/docs/16_chaos_c_extensions_development)

[**API Reference**](https://chaos-lang.org/docs/api)

[**Template for Chaos C Extension Developers**](https://github.com/chaos-lang/template)

[**Documentation Repository**](https://github.com/chaos-lang/chaos-lang.org)

[**Bug Tracker**](https://github.com/chaos-lang/chaos/issues)

[**Occultist Dependency Manager**](https://github.com/chaos-lang/occultist)
