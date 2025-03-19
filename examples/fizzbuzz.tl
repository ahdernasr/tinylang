// FizzBuzz implementation in TinyLang
fn fizzbuzz(n) {
    if (n % 15 == 0) {
        return "FizzBuzz";
    } else if (n % 3 == 0) {
        return "Fizz";
    } else if (n % 5 == 0) {
        return "Buzz";
    } else {
        return toString(n);
    }
}

// Main program
let i = 1;
while (i <= 100) {
    print(fizzbuzz(i));
    i = i + 1;
}
