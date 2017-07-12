#include <iostream>

#include "etool/slices/memory.h"

#include "mico/tokens.h"
#include "mico/lexer.h"

#include "mico/objects/objects.h"
#include "mico/parser.h"
#include "mico/eval/tree_walking.h"
#include "mico/repl.h"

int run_repl( )
{
    mico::repl::run( );

    return 0;
}

using namespace mico;

/*

let fac = fn(val) {
    let impl = fn( val, acc ) {
        if val > 1 {
            impl( val - 1, acc * val )
        } else {
            acc
        }
    }
    impl( val, 1 )
}

let fib = fn( n ) {
    let x = fn(count) {
        if count > 0 {
            fn( ){ x(count - 1) }
        } else {
            fn( ){ return 15 }
        }
    }
    let impl = fn( a, b, n ) {
        if n > 0 {
            impl( b, a + b, n -1 )
        } else {
            x(100)( );
            a
        }
    }
    impl(0, 1, n)
}

let fib = fn( n ) {
    let impl = fn( a, b, n ) {
        if(n > 0) {
            impl( b, a + b, n -1 )
        } else {
            a
        }
    }
    impl(0, 1, n)
}

let calls = {
    true: fn( n ) {
        let impl = fn( a, b, n ) {
            if(n > 0) {
                impl( b, a + b, n -1 )
            } else {
                a
            }
        }
        impl(0, 1, n)
    },
    false: fn( n ) { n; }
}

let t = fn( x ) {
    let m = fn( y ) {
        y + 10
    }
    m( x + 1 )
}

let p = {"x": 0, "y": 0}
let t = {p: 1000}
t[{"x": 0, "y": 0}]
p["y"] = 1000
p["y"] = 0

let a = [[100], 1,2,3,4]
let m = {a: "Hello!"}
m[[[100], 1,2,3,4]]

let a = [1,2,3,4,5]
a[0] = a
let t = {a: 100}

let sum = fn( count ) { // start
    let impl = fn( a, new ) { /// internal
            r(10)
        if( new == 0 ) {
            a
        } else {
            return impl( a + 1, new - 1 )
        }
    }
    impl( 0, count )
}

let gr = fn(name) { fn( hello ) { hello + name } }
let t = gr("hello T")
let r = gr("hello R")
t( )

let x = fn(count){if(count > 0) { return x(count - 1) } else {0} }
let x = fn(count){if(count > 0) { return x(count - 1) + 1; 60 } 0 }
let x = fn( ){ x( ) }

let fac = fn( x ) { if(x > 1) { x * fac(x - 1)  } else { 1 } }
let fib = fn(x) {
    if (x == 0) { 0 } elif ( x==1 ) { 1 }
    else { fib(x - 1) + fib(x - 2); }
};

/// it doesnt leak anymore  =)
let  z = fn(c) { let test = fn( ) { let a = [0, 1,2,3,4,5,6,7,8,9]; let b = a; let int = fn( ) { return a; }; let intA = int; let intB = int; return int } let copy_test = test( ); if(c>0) { test( ); return z(c - 1); } else { return copy_test; } }

// Here is an array containing two hashes, that use strings as keys and integers
// and strings as values
let people = [{"name": "Anna", "age": 24}, {"name": "Bob", "age": 99}];
// We can also access hash elements with index expressions:
let getName = fn(person) { person["name"]; };
// And here we access array elements and call a function with the element as
// argument:
getName(people[0]); // => "Anna"
getName(people[1]); // => "Bob"

let create_array = fn( ) {
    let a = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
    fn( ) { a }
}
let array = create_array( )
array( )[0] =
array( )[1] =
array( )[2] =
array( )[3] =
array( )[4] =
array( )[5] =
array( )[6] =
array( )[7] =
array( )[8] =
array( )[9] = 0.001
array( )

let test = fn( ) {
    let a = [0, 1,2,3,4,5,6,7,8,9];
    let b = a;
    let i = fn( ) { [0, 1,2,3,4,5,6,7,8,9] }
    let int = fn( ) { a }
    int
}
let z = fn( c ) {
    let test = fn( ) {
        let a = [0, 1,2,3,4,5,6,7,8,9];
        let b = a;
        let i = fn( ) { [0, 1,2,3,4,5,6,7,8,9] }
        let int = fn( ) { a }
        int
    }
    let t = test( )
    if c > 0  {
        test( )
        z(c-1)
    } else {
        test( )
    }
}

let t = fn( ) { 10 }

let g = fn (name) {
    {
      false: fn( ) { "This is not true, " + name },
      true: fn( )  { "This is true, " + name }
    }
}
let f = g("first")
let a = [[f[false]], [f[true]]]
let a = [[f[false]], [{0: f[true]}]]
let a = [f[false], f[true]]

//f[false] = 0
let r = fn(  ){ let bu = f; bu[true] }
let m = r( )

let test = fn( ) {
    let a = [0, 1,2,3,4,5,6,7,8,9];
    let t = test
    fn( ) { a }
}
let m = fn( ) {
    let r = test( )
    r
}
let r = test( )

let  z = fn(c) {
    if(c>0) { test(); z(c - 1) } else { 0 }
}

let zz = fn( nn ) {
    let impl = fn( n ) {
        if( n > 0 ) {
            test( )
            test( )( )
            impl( n - 1 )
        } else {
            test( )
        }
    }
    impl( nn )
}

let x = "x"
let y = "y"
let z = "z"

let point = fn( x, y, z ) { {"x": x, "y": y, "z": z} }
let table = {
    point(0, 0, 1): 0.1,
    point(0, 1, 0): 0.2,
    point(0, 1, 1): 0.3,
    point(1, 0, 0): 0.4,
    point(1, 0, 1): 0.5,
    point(1, 1, 0): 0.6,
    point(1, 1, 1): 0.7,
}

*/

void run_file( )
{

}

int main_lex( );

int main( int argc, char * argv[ ]  )
{

//    auto s = std::make_shared<int>(100);

//    std::weak_ptr<int> w1 = s;
//    std::weak_ptr<int> w2 = s;
//    std::weak_ptr<int> w3 = w2;

//    auto s1 = w3.lock( );

//    std::cout << w1.use_count( ) << "\n";
//    std::cout << w3.use_count( ) << "\n";
//    std::cout << s1.owner_before( w3 ) << "\n";


//    return 0;
    //return main_lex( );

    return run_repl( );
}

