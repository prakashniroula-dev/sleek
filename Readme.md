# Sleek
A statically-typed, sleek, ergonomic, expressive, and modern programming language designed for clarity and performance.
> Currently in early development (work in progress), so expect breaking changes and incomplete features. The design is still evolving, and feedback is welcome!
<hr/>

Check out the [Docs](https://prakashniroula-dev.github.io/sleek)

## First MWP (Minimum Working prototype) -- Achieved

Compile/Transpile and run this piece of code

```sleek
fn main() {
  print("Hello Sleek!", " (", 5, 1, 3, 3, "K )");
}
```

| Todos |
---

- [x] Lexer - lex tokens
  - [x] keywords - `fn`
  - [x] symbols - `(){};"`
  - [x] identifiers - `print` | `main`

- [x] Parser - parse to AST
- [x] Transpiler - transpile to C code

```yaml
# AST preview
[function_defn]:
  name: "main"
  return_type: 2 (void)
  body:
    [function_call]:
      name: "print"
      args:
        arg 0:
          [literal]:
            type: 3 (string)
            value: "Hello Sleek!"
        arg 1:
          [literal]:
            type: 3 (string)
            value: " ("
        arg 2:
          [literal]:
            type: 4 (integer)
            value: 5
        arg 3:
          [literal]:
            type: 4 (integer)
            value: 1
        arg 4:
          [literal]:
            type: 4 (integer)
            value: 3
        arg 5:
          [literal]:
            type: 4 (integer)
            value: 3
        arg 6:
          [literal]:
            type: 3 (string)
            value: "K )"
```

## How to run

1. Clone the repo
2. Build the project using `make`
3. Run the compiled binary with `./sleek`