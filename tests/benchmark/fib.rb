def fib(n)
    n <= 2 ? 1 :  fib( n - 1 ) + fib( n - 2 )
end
puts fib(40)
