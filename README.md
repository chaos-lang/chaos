# Chaos

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
</p>
<p align="center">
    Chaos is a purely functional programming language that achieves zero cyclomatic complexity.
</p>

## Installation

Install the requirements:

```
sudo apt install build-essential bison flex
```

Compile the interpreter:

```
make
```

Install the `chaos` binary system-wide:

```
sudo make install
```

## Usage

### Interactive Shell

```
mertyildiran@Corsair:~/Documents/chaos$ chaos
Chaos Language 0.0.1-alpha (Dec 23 2019 04:18:23)
GCC version: 7.4.0 on linux
Turn chaos into magic!

### print "hello world"
hello world
### 
```

### Program File as Command-line Argument

```
mertyildiran@Corsair:~/Documents/chaos$ chaos dev.chs
8
1231232
asdasdasd
qweqweqw
hey
true
hey
7
5.400000
string2
string4
[start] => var1 => var3 => var4 => var5 => var6 => var7 => var8 => [end]
[end] => var8 => var7 => var6 => var5 => var4 => var3 => var1 => [start]
```

## Data Types

### Primitive Data Types

#### Boolean

There are two keywords for **Blooean** data type: `bool` or `boolean`

```
bool var1 = true
boolean var2 = false
var1 = false
var2 = true
print var1
print var2
```

#### Number

There are two keywords for **Number** data type: `num` or `number`

```
num var5 = 5
var5 = 7
print var5
number var6 = 3.2
var6 = 5.4
print var6
```

#### String

There are two keywords for **String** data type: `str` or `string`

```
str var7 = 'string1'
var7 = 'string2'
print var7
string var8 = 'string3'
var8 = 'string4'
print var8
```

## Mathematical Expressions

```
### 3 + 5
8
### 145 + 24.8
169.800003
### 0.32 + 0.42
0.740000
### (3 + 5) * 2
16
```

## Keywords

### print

To print a variable or a value to `stdout` simply use `print` keyword:

```
### print 77
77
### print "hello world"
hello world
### print 'hello world'
hello world
### str hello = "world"
### print hello
world
```

### del

`del` keyword deallocates the memory region that previously allocated for the given variable:

```
del var2
```

### exit

`exit` keyword terminates the program:

```
### exit
You have chosen the order!
```

### quit

`quit` alias of `exit`:

```
### quit
You have chosen the order!
```

### symbol_table

`symbol_table` command shows all of the variables currently being tracked by Symbol Table.
This keyword is only useful for the language developers:

```
[start] => var1 => var3 => var4 => var5 => var6 => var7 => var8 => [end]
[end] => var8 => var7 => var6 => var5 => var4 => var3 => var1 => [start]
```
