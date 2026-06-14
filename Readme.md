# Sleek
A statically-typed, sleek, ergonomic, expressive, and modern programming language designed for clarity and performance.
> Currently in early development (work in progress), so expect breaking changes and incomplete features. The design is still evolving, and feedback is welcome!
<hr/>

Check out the [Docs](https://prakashniroula-dev.github.io/sleek)

## Todos for first MWP (Working prototype) :

Compile and run this piece of code

```sleek
fn main() {
  print("Hello Sleek");
}
```

| Todos |
---

- [x] Lexer - lex tokens
  - [x] keywords - `fn`
  - [x] symbols - `(){};"`
  - [x] identifiers - `print` | `main`

- [x] Parser - parse to AST

```yaml
# AST preview
[function_defn]
  name: "main"
  args: 0
  body: (linked_list, length=1)
    [statement]
      [call]
        body:
          fn_name: "print"
          args: (linked_list, length=1)
            0: [literal]
              type: string
              body: "Hello World"
```

## How to run

1. Clone the repo
2. Build the project using `make`
3. Run the compiled binary with `./sleek`