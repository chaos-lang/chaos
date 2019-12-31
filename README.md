# Chaos

<p align="center">
  <img src="https://i.ibb.co/7V0mKxf/chaos-logo-small.png" alt="Logo" height="300px"/>
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
</p>
<p align="center">
    Chaos is a purely functional programming language that achieves zero cyclomatic complexity.
</p>

### Influenced by

 - TypeScript's type safety
 - Python's syntax, modules and integrity
 - JavaScript's availability
 - Ruby's loops
 - PHP's dedication to server-side
 - Haskell's immutability
 - C's speed
 - NumPy's matrix arithmetics
 - Perl's regex engine

### Unique Features

 - There are no control structures. (no `if..else`, no `switch..case`) Decision making only possible on function returns: `func myFunc(a, b){ 'x > 0' : f1(x), 'default' : f2(x) }:`
 - A single unit test is enough to have 100% coverage on functions, always.
 - Warns you about untested functions in the executed program.
 - Chaos language is not object-oriented. So everything is done by functions and data types.
 - Immutability everywhere.
 - No `while` or `when` keywords. Loops are predetermined. There is also `INFINITE` keyword to achieve infinite loops if it's necessary.
 - Single source of errors and unique exit codes for each one of them. So the errors are catchable by other programs.
 - Language's itself forces you to write less error-prone code.
 - Eliminated segmentation fault possibility using strict memory management and predictions.
 - Integrity via Chaos C extensions.

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
mertyildiran@Corsair:~/Documents/chaos$ chaos dev.kaos
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
### bool var1 = true
### boolean var2 = false
### var1 = false
### var2 = true
### print var1
false
### print var2
true
```

#### Number

There are two keywords for **Number** data type: `num` or `number`

```
### num var5 = 5
### var5 = 7
### print var5
7
### number var6 = 3.2
### var6 = 5.4
### print var6
5.400000
```

#### String

There are two keywords for **String** data type: `str` or `string`

```
### str var7 = 'string1'
### var7 = 'string2'
### print var7
string2
### string var8 = 'string3'
### var8 = 'string4'
### print var8
string4
```

### Complex Data Types

#### Arrays

There are two keywords for **Array** data type: `list` or `array`

```
### array a = [1, 2, 3]
### print a
[1, 2, 3]
### list b = [true, false]
### print b
[true, false]
### list c = [3.2, 345.1665]
### print c
[3.200000, 345.166504]
### list d = ['a', 'b', 'c']
### print d
['a', 'b', 'c']
### array e = ["A", "B", "C"]
### print e
['A', 'B', 'C']
### list f = ['A', "asdasdad12312", 1232, 435.16, true, false]
### print f
['A', 'asdasdad12312', 1232, 435.160004, true, false]
```

Multi-line array definition:

```
### array a = [
    'foo',
    'bar',
    'baz'
]
```

Accessing array elements:

```
### list d = ['a', 'b', 'c']
### print d[2]
c
### print d[1]
b
### print d[0]
a
### print d[-1]
c
### print d[-2]
b
### print d[-3]
a
```

Updating array elements:

```
### list a = [1, 2, 3]
### a[0] = 5
### print a
[5, 2, 3]
```

Deleting array elements:

```
### list g = [1, 2, 3, 4, 5]
### print g
[1, 2, 3, 4, 5]
### del g[1]
### print g
[1, 3, 4, 5]
```

#### Typed Arrays

It's possible to create typed arrays in Chaos language
by prefixing `list` or `array` keywords with a [Primitive Data Type](#primitive-data-types):

```
### bool list arr1 = [true, false, true]
### print arr1
[true, false, true]
### number list arr2 = [1, 2, 63.3, 12321.1515]
### print arr2
[1, 2, 63.299999, 12321.151367]
### string array arr3 = ['A', "asdasdaqs", 'asdasd123123', "."]
### print arr3
['A', 'asdasdaqs', 'asdasd123123', '.']
```

#### Dictionaries

There are two keywords for **Dictionary** data type: `json` or `dict`

```
### dict a = {'a': 1, 'b': 2}
### print a
{'a': 1, 'b': 2}
### dict b = {'a': 1, 'b': 2, 'c': 3}
### print b
{'a': 1, 'b': 2, 'c': 3}
### json c = {"a": 1, "b": 2}
### print c
{'a': 1, 'b': 2}
### json d = {"a": 1, "b": 2, "c": 3}
### print d
{'a': 1, 'b': 2, 'c': 3}
```

Multi-line dictionary definition:

```
### dict a = {
    'a': 1,
    'b': 2,
    'c': 3
}
```

Accessing dictonary elements:

```
### dict a = {'a': 1, 'b': 2, 'c': 3}
### print a['a']
1
### print a["b"]
2
```

Updating dictonary values:

```
### dict a = {'a': 1, 'b': 2, 'c': 3}
### a['a'] = 5
### print a
{'a': 5, 'b': 2, 'c': 3}
```

Deleting dictionary keys:

```
### json d = {"a": 1, "b": 2, "c": 3}
### print d
{'a': 1, 'b': 2, 'c': 3}
### del d['a']
### print d
{'b': 2, 'c': 3}
```

#### Typed Dictionaries

It's possible to create typed dictionaries in Chaos language
by prefixing `json` or `dict` keywords with a [Primitive Data Type](#primitive-data-types):

```
### bool dict dict1 = {'a': true, 'b': false}
### print dict1
{'a': true, 'b': false}
### number dict dict2 = {'a': 1, "b": 2, 'c': 63.3, 'd': 12321.1515}
### print dict2
{'a': 1, 'b': 2, 'c': 63.299999, 'd': 12321.151367}
### string json dict3 = {'a': 'A', 'b': "asdasdaqs", 'c': 'asdasd123123', 'd': "."}
### print dict3
{'a': 'A', 'b': 'asdasdaqs', 'c': 'asdasd123123', 'd': '.'}
```

## Immutability

Every variable in Chaos language is **immutable by default**.
When you assign a variable to another variable, the language's itself
creates a **deep copy** of that variable. That means; the new(*left-hand*) variable will
live in a completely different memory region than the old(*right-hand*) variable.

Here are some examples of immutability on Chaos language:

```
### bool a1 = true
### bool b1 = false
### b1 = a1
### print b1
true
### a1 = false
### print b1
true

### num a2 = 3.5
### num b2 = 7.1
### b2 = a2
### print b2
3.500000
### a2 = 1.2
### print b2
3.500000
```

Immutability is also valid on new variable creation:

```
### bool a3 = true
### bool b3 = a3
### print b3
true
### a3 = false
### print b3
true

### str a4 = 'asdasda'
### str b4 = a4
### print b4
asdasda
### a4 = 'qwqweqwe'
### print b4
asdasda
```

Arrays and the elements of arrays are also immutable:

```
### num x = 5
### array y = [x, 2, 3]
### print y
[5, 2, 3]
### x = 32
### print x
32
### print y
[5, 2, 3]

### list z = [1, 2, 3]
### list t = z
### print z
[1, 2, 3]
### print t
[1, 2, 3]
### z[0] = 5
### print z
[5, 2, 3]
### print t
[1, 2, 3]

### list y2 = [1, 2, 3]
### num x2 = 5
### y2[0] = x2
### print x2
5
### print y2
[5, 2, 3]
### x2 = 7
### print x2
7
### print y2
[5, 2, 3]
```

Dictionaries and the values of dictionaries are also immutable:

```
### num k = 5
### dict q = {'a': k, 'b': 2, 'c': 3}
### print q
{'a': 5, 'b': 2, 'c': 3}
### k = 32
### print k
32
### print q
{'a': 5, 'b': 2, 'c': 3}

### dict n = {'a': 'foo', 'b': 'bar', 'c': 'baz'}
### dict m = n
### print n
{'a': 'foo', 'b': 'bar', 'c': 'baz'}
### print m
{'a': 'foo', 'b': 'bar', 'c': 'baz'}
### n['a'] = 'alt'
### print n
{'a': 'alt', 'b': 'bar', 'c': 'baz'}
### print m
{'a': 'foo', 'b': 'bar', 'c': 'baz'}

### dict q2 = {'a': 1, 'b': 2, 'c': 3}
### num k2 = 5
### q2['a'] = k2
### print k2
5
### print q2
{'b': 2, 'c': 3, 'a': 5}
### k2 = 7
### print k2
7
### print q2
{'b': 2, 'c': 3, 'a': 5}
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
This keyword is only useful for the developers of language's itself:

```
[start] => var1 => var3 => var4 => var5 => var6 => var7 => var8 => [end]
[end] => var8 => var7 => var6 => var5 => var4 => var3 => var1 => [start]
```

## Extras

### Program File Extension

Recommended extension for a Chaos program file is: `.kaos`
