# The Rulma Programing language
<p align="center">
    <img src="logo_full.svg" width="512" alt="The Rulma Programing language logo">
</p>
The Rulma Programing language is an idea for a general purpose systems programing language like (C/C++, Rust and Zig). Our goal currently is to make a proof of concept compiler.

## Syntax

```rulma
let a = 10
let b = 5

space Math {
    fn sum(_a, _b) {
        ret _a + _b
    }
}

fn foo() {
    let c
    c = Math.sum(a, b)

    fn multiply(_a, _b) {
        ret _a * _b
    }

    let d = multiply(a, b)
}

```

## License

MIT License

Copyright (c) 2024 Mohammedi Mohammed Djawad

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
