// Closures and higher-order functions in TinyLang

// Simple closure example
fn make_counter() {
    let count = 0;
    return fn() {
        count = count + 1;
        return count;
    };
}

let counter1 = make_counter();
let counter2 = make_counter();

print("Counter 1: " + toString(counter1()));
print("Counter 1: " + toString(counter1()));
print("Counter 2: " + toString(counter2()));
print("Counter 1: " + toString(counter1()));

// Function that returns a function
fn make_multiplier(factor) {
    return fn(x) {
        return x * factor;
    };
}

let double = make_multiplier(2);
let triple = make_multiplier(3);

print("double(5) = " + toString(double(5)));
print("triple(5) = " + toString(triple(5)));

// Recursive closure
fn make_factorial() {
    return fn(n) {
        if (n <= 1) {
            return 1;
        }
        return n * make_factorial()(n - 1);
    };
}

let factorial = make_factorial();
print("factorial(5) = " + toString(factorial(5)));
