// Loop benchmark
let outer = 0;
while (outer < 1000) {
    let inner = 0;
    while (inner < 1000) {
        let temp = outer * inner;
        inner = inner + 1;
    }
    outer = outer + 1;
}
print("Loop benchmark completed");
