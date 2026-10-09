int add(int a, int b) {
    return a + b;
}

float calculate(float x, float y) {
    if (y == 0.0) {
        return 0.0;
    }

    return x / y;
}
// arg types: int, float, char, bool, var, type (eg. Person), pointer, reference (as a pointer), [no args]
// return types: int, float, char, bool, var, type (eg. Person), pointer, void

// Struct (type)
type Person {
    string name;
    int age;
};

int main() {
    // Primitive types
    int count = 42; //32bit
    float temperature = 98.6; //32bit
    char letter = 'A'; //8bit
    bool running = true; //1bit
    // if input exceeds bit range, compile-time error for out-of-range constant value
    // more types: i8, i16, i32, i64, u8, u16, u32, and u64

    int x1 = 2147483648;    // Compile-time error
    int y1 = 10 / 3;        // 3, if integer division truncates toward zero
    float z1 = 10.0 / 3.0;  // Approximately 3.3333333

    // Manual bit allocation
    var(3) val = -3; //3bit in this case, signed
    unsigned var(3) uVal = 5; //3bit in this case, unsigned
    // min size is 1, max size is 64. size of 1 must be unsigned

    // String
    string msg = "Hello";

    // Constants
    const int limit = 10;
    const var(5) max = 15;

    // Unsigned variables
    unsigned int uInt = 74;

    // Arithmetic operators
    int a = 10 + 5;
    int b = 10 - 5;
    int c = 10 * 5;
    int d = 10 / 5;
    int e = 10 % 3;

    // Assignment operators
    a += 5;
    b -= 2;
    c *= 3;
    d /= 2;
    e %= 2;

    // Increment / decrement
    count++;
    count--;

    // Comparison
    bool result1 = a == b;
    bool result2 = a != b;
    bool result3 = a < b;
    bool result4 = a > b;
    bool result5 = a <= b;
    bool result6 = a >= b;

    // Logical operators
    bool result7 = (a > 0 && b > 0);
    bool result8 = (a > 0 || b > 0);
    bool result9 = !running;

    // Bitwise operators
    int bits = a & b;
    bits = a | b;
    bits = a ^ b;
    bits = ~a;
    bits = a << 2;
    bits = a >> 1;

    // Conditional
    if (count > 50) {
        print("Large\n");
    } else if (count == 50) {
        println("Exactly 50");
    } else {
        println("Small");
    }

    // Ternary operators
    int maximum = (a > b) ? a : b;

    // While loop
    int i = 0;

    while (i < 10) {
        println(i);
        i++;
    }

    // Do-while loop
    do {
        count--;
    } while (count > 0);

    // For loop
    for (int j = [0:10:1]) {
        print("j: ${j}\n");
    }

    // Break / continue
    for (int j = [0:100:1]) {
        if (j == 25) {
            break;
        }

        if (j % 2 == 0) {
            continue;
        }

        if (j != 0) {
            print(", ");
        }
        print("${j}");
    }

    // Arrays
    int numbers[5] = {1, 2, 3, 4, 5};
    int numbers2[] = {1, 2, 3, 4, 5};
    int numbers3[5]; // auto sets to {0, 0, 0, 0, 0}
    int numbers4[5] = {1, 2, 3}; // auto sets to {1, 2, 3, 0, 0}

    // Access
    int first = numbers[0];

    // Function call
    int sum = add(10, 20);

    // Nested scopes
    {
        int scopedValue = 123;
        println(scopedValue);
    }

    // Null pointer
    int* pointer = nullptr;

    // Address / dereference
    int value = 10;
    pointer = &value;
    int dereferenced = *pointer;
    *pointer += 2;

    // Struct (type) (cont.)
    Person person;
    person.name = "Adi";
    person.age = 18;

    // Pointer member access;
    Person* personPtr = &person;
    println(personPtr->name);

    // Namespace
    print("Hello\n");

    // Switch (Compare)
    compare (count) {
        case (0) {
            println("Zero");
        }
        case (1) {
            println("One");
        }
        default {
            println("Other");
        }
    }

    return 0;
}