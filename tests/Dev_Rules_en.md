# inicpp Development Rules

This file records the project rules that should be followed when changing
`inicpp.hpp`, examples, documentation, or tests.

## Compatibility Rules

- Keep `inicpp.hpp` as a single-header library.
- Keep C++11 support. Do not use language or library features that require a
  newer standard unless the project explicitly raises its minimum standard.
- Do not introduce third-party dependencies.
- Preserve the public API and current README/example usage unless an API change
  is explicitly approved.
- `README.md` and `example/main.cpp` must continue to compile with strict C++11
  settings.
- Avoid large rewrites. Prefer scoped internal improvements that match the
  existing library shape.

## Header Integration Rules

- Do not use reserved identifiers for include guards, macros, or internal names.
- Avoid exporting generic macros or global helper types from `inicpp.hpp`.
- Keep debug-only helpers inside the `inicpp` namespace.
- Any public macro should use an `INICPP_` prefix.
- Include only the standard headers that are needed by the header implementation.

## INI Behavior Rules

- Preserve documented parsing behavior:
  - UTF-8 BOM at the beginning of the file is ignored.
  - Section headers may have leading whitespace and inner edge whitespace.
  - Key-value delimiters are `=` and `:`.
  - Empty values are valid.
  - Full-line comments use `;` or `#`, including after leading whitespace.
  - Inline comments are stripped only when preceded by whitespace and not inside
    quoted or escaped values.
  - Comment markers inside quoted or escaped values are preserved.
- Duplicate keys in the same section use the last parsed value.
- Duplicate section headers are merged, and repeated keys use the last parsed
  value.
- Do not silently add new INI dialect features, such as multiline values, bare
  keys, quote removal, arrays, includes, or nested sections, without updating
  tests and README.

## Write Safety Rules

- `parse()` must not create files. Reading and parsing should be side-effect
  free when the target file does not exist.
- Write paths may create a missing target file when the caller performs a write.
- File replacement must use a temporary file and backup/restore path rather than
  truncating the original file in place.
- On write failure, keep the original file recoverable and remove temporary
  files when possible.
- Preserve existing LF or CRLF line endings when rewriting a file.
- `set(section, key, "")` must write an empty value as `key=`.
- `setComment()` must support missing section/key by creating the key with an
  empty value.
- Updating a comment should remove an immediately preceding old comment line
  when a new comment is supplied.

## Test Rules

- The main verification command is:

  ```sh
  make -C tests clean test
  ```

- Also run whitespace checks before committing:

  ```sh
  git diff --check
  ```

- `tests/Makefile` must keep `example_compile` as the first test dependency so
  README/example usage regressions are caught early.
- New behavior must have a focused regression test before implementation.
- Keep tests under `tests/`. Negative compile tests belong under
  `tests/negative/`.
- Negative compile tests must fail compilation to pass the test target.
- Do not commit generated test binaries, object files, logs, `.ini` files,
  `.inicpp.tmp` files, or `.inicpp.bak` files.
- If a test needs platform-specific behavior, keep the portable path active and
  isolate platform-specific code with preprocessor guards.

## Documentation Rules

- Update `README.md` when supported syntax, write behavior, public API behavior,
  or example usage changes.
- Keep README claims aligned with tested behavior. Do not claim complete INI
  compatibility beyond what the project supports and tests.
- Keep `example/main.cpp` aligned with README usage and C++11 aggregate
  initialization rules.

## Git Rules

- Commit only source, documentation, and test files that are intentionally part
  of the change.
- Do not commit generated build/test artifacts.
- Do not rewrite history, reset, or discard local changes unless explicitly
  requested.
- Before committing, verify the branch, staged file list, test output, and
  whitespace check output.
