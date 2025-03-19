// Control flow examples in TinyLang

// If-else statements
let x = 42;
if (x > 50) {
    print("x is greater than 50");
} else if (x > 30) {
    print("x is greater than 30 but not 50");
} else {
    print("x is 30 or less");
}

// While loops
let i = 0;
while (i < 5) {
    print("i = " + toString(i));
    i = i + 1;
}

// For loops (desugared to while)
for (let j = 0; j < 3; j = j + 1) {
    print("j = " + toString(j));
}

// Nested loops
let outer = 0;
while (outer < 3) {
    let inner = 0;
    while (inner < 2) {
        print("outer=" + toString(outer) + ", inner=" + toString(inner));
        inner = inner + 1;
    }
    outer = outer + 1;
}

// Break and continue (simplified)
let k = 0;
while (k < 10) {
    if (k == 3) {
        k = k + 1;
        continue; // Skip this iteration
    }
    if (k == 7) {
        break; // Exit loop
    }
    print("k = " + toString(k));
    k = k + 1;
}
