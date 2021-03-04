
# Radix  
  
**Radix** language 32-bit compiler to **Netwide Assembler (NASM)** for linux  
  
**Important:** install nasm package before running Radix compiler  
  
  
## Compilation  
  
 	./radix [path-to-main-file] [program-name] 

Running program generaters 2 major files:  
  
1. program-name.prad  
  
    Processed .rad file generated by preprocessor.  
    it shows how compiler views the code.  
  
2. program-name.asm  
  
    Compiled .prad file generated by compiler.  
      
Run compiled program:  
  
 	./[program-name]

## Documentation 

### Comments

Just put "`" at the start of the line
  
### Type Specifiers  
  
| Type Specifier                | Usability                                                      |  
|-------------------------------|----------------------------------------------------------------|  
| i8                            | Stores 1 byte value                                            |  
| i16                           | Stores 2 byte value                                            |  
| i32                           | Stores 4 byte value, Stores memory address                     |  
  
	  i32 a = 97;
	  i32 a = 'a';
  
More complex expressions are also supported

	i8 a = 97 + 5 - 3 + 1;
	` Compiler uses compile time expression evaluation, so following code is translated to:
	i8 a = 100;
	` Than it's translated to NASM. However expressions containing variables can not be fully evaluated
	i16 a = 7 + 8;
	i32 b = a + 5 - 4;
	` This is simplified to following
	i16 a = 15;
	i32 b = a + 5 - 4;
	` Because compiler evaluates expression from left to right it can not simplify 5 - 4
	i32 b = a + (5 - 4);
	` This can help so now its translated to
	i32 b = a + 1;

Here is a list of all available operators

| Operator   | Function                                         | return          | usage     | precedence |
|------------|--------------------------------------------------|-----------------|-----------|---|
| =          | Stores rightside value into leftside variable    | rightside value | a = b     | 2 |
| +          | Adds two values                                  | value           | a + b     | 12 |
| -          | Subtracts right value from left value            | value           | a - b     | 12 |
| $          | Returns variable memory address                  | address         | a$        | 15 |
| ?          | Dereferences memory pointer                      | value           | a ? i32   | 15 |
| :          | Calls function                                   | value           | f : a, b  | 0 |
| &          | Logical and                                      | value           | a & b     | 7 |
| ?=         | Dereference and assign                           | rightside value | a ?=i8 b  | 2 |
  
### Functions

Functions are defined by `fun` keyword followed by function name and arguments. However, it can take 0 arguments

	fun no_args {}
	fun with_args : i32 a, i16 b {}

The entry function is `fun start {}`
Calling functions is possible by following code

	fun callme {}
	
	fun start {
		callme:;
	}

Operator `:` is used for function calling. Passing arguments is also possible

	fun callme : i16 a, i8 b, i8 c {
		i16 sum = a + b + c;
	}
	
	fun start {
		callme : 'a', 7, 99;
	}

Functions can also have return types

	fun with_ret
	type : i32 {
		i16 b = 5;
		ret : b;
	}

Functions with no type definition always return 0 so they can still  be used in expressions

	fun with_ret
	type : i32 {
		i16 b = 5;
		ret : b;
	}
	
	fun no_ret {}
	fun no_ret_with_ret { ret; }

	fun start {
		i32 a = (with_ret:) + (no_ret:);
		` Its important to put function calls in parentheses as : has lowest precedence
	}

### if, loop, break , continue, skip, exit

you can loop unconditionally any code segment by putting it into loop {}

	loop {
		i32 a = 1;
	}

However, you can addcondition to it so you can break out of it at certain moment
	
	i32 a = 5;
	
	loop if : a {
		a = a - 1;
	}

This loop will stop after 5 cycles
you can put loop at the end too if you want

	if : a loop {}

If you want code to execute for just once if condition is met just remove "loop"

	if : a {}

Also all : fun, type, loop, if keywoards can be combined to create

	type : i32 fun : i8 a, i16 b loop if : 1 {}

The program will enter function as many times as condition is met but will break if ret is called. But probably can cause alot of bugs? its safer to use with functions with no ret type and no args

Sometimes its also important to break loop. To do so simply call break;

	loop { break; }

But Sometimes its even more important to break from multipe loops. To do so pass how many nested loops you want to break from as argument.

	loop { loop { loop { break : 3; } } }

Same syntax goes with continue which can make program jump at start of loop

	loop { loop { loop { continue : 2; } }

You can not only break from a loop but also from regular scopes

	{ { { { { skip : 3; } } } } }

By skipping scopes you just jump at the end of the scope

And finally you can exit program at any time. Just call `exit` to do so

	exit;

But it can take no args as you can not exit more then one program. Obviously

### in/out

You can write simple input / output by calling `in : a` or `out : a`, but it can only read and write in ascii format, so it means that inputting '1' is really '49'. It will read and write number of bytes according to variable type. If constant is used then 4 bytes are read / written

### stack, scopes

Each stack frame is created when program enters new function. Recursive functions Create new stack as they enter itself again. You can not access variables from another stack. All variables are destroyed at the end of the scope. You can define variables with the same name as many times as you wish. The last defined variable with same name will be used when trying to access it
  
### macros

Use `#add` macro to include files from the same or different directory.

	------ ./helper.rad ------
	fun sum : i32 a  
	type : i32 {  
	    if : a { ret: a + (sum: a - 1); }  
	  
	    ret : a;  
	}

	------ main.rad ------
	#add helper.rad  
  
	fun start {  
	    out : (sum: 3) + '0';  
	}

### arrays, reference, dereference

Variable references are stored in `i32`
To store reference use `$` operator

	i8 a = 'a';
	i32 ref = a$;

To dereference and get value dereference it by `?` operator followed by type specifier

	a = ref ? i8;

To change variable value from its reference use `?=` operator followed by type specifier

	ref ?=i8 (mul: 5, 4) + 2;

Arrays are declared using arr keyword followed by type specifier, variable name and size as first argument

	arr i16 a : 100;

You can directly change second member of array by following code

	(a$ + 2) ?=i16 98;
	` Notice +2 is used as i16 is 2 bytes long
