// Arithmetic benchmark
let sum = 0;
let i = 0;
while (i < 1000000) {
    sum = sum + i;
    i = i + 1;
}
print("Sum: " + toString(sum));
