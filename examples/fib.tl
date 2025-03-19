// Fibonacci implementation in TinyLang
fn fib(n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

// Iterative version
fn fib_iter(n) {
    if (n <= 1) {
        return n;
    }
    
    let a = 0;
    let b = 1;
    let i = 2;
    
    while (i <= n) {
        let temp = a + b;
        a = b;
        b = temp;
        i = i + 1;
    }
    
    return b;
}

// Test both implementations
print("Recursive Fibonacci:");
let i = 0;
while (i <= 10) {
    print("fib(" + toString(i) + ") = " + toString(fib(i)));
    i = i + 1;
}

print("\nIterative Fibonacci:");
i = 0;
while (i <= 10) {
    print("fib_iter(" + toString(i) + ") = " + toString(fib_iter(i)));
    i = i + 1;
}
