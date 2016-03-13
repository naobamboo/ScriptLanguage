# ScriptLanguage

ScriptLanguage is original language and interpreter for personal-schooling.

## QuickStart

- Clone the repo: $ git clone https://github.com/naobamboo/ScriptLanguage.git
- Change directory: $ cd ScriptLanguage
- Install program: $ make
- Run: $ ./ScriptLanguage */path/to/file*
(default is ./sample.txt)

## Usage
You can run these codes.

```
// sample.txt
// FizzBuzz example
num  = 15;
println("FizzBuzz 1 to ", num);

i = 1;
while(i <= num) {
	if ((i % 15) == 0) {
		println("FizzBuzz");
	} elsif ((i % 3) == 0) {
		println("Fizz");
	} elsif((i % 5) == 0) {
		println("Buzz");
	} else {
		println(i);
	}
	i = i + 1;
}

// Function Test
function test(x, y) {
	println("function: ", x + y);
}

c = 5;
d = 10;
test(c, d);

```

## Detail
ScriptLanguage supports funcion as below:

- Number typing (int, ant float)
- String typing
- Symbol typing
- Expression +, -, *, /, and %
- Comparison ==, !=, <, >, <=, and >=
- Built-in function call (now println only)
- User function (declare and call)
- While statement
- If (elsif, else) statement
- Comment (one line)
