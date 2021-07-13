<?php
function fib($n)
{
    return $n < 3 ? 1 : fib($n - 1) + fib($n - 2);
}

echo fib(40).PHP_EOL;
?>
