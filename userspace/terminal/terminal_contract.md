# XyrisOS Userspace Terminal Contract

## Scope

The terminal module is the userspace adapter between a platform-provided
character stream and the Xyris shell. It provides line editing, prompt output,
and command dispatch.

## Interface boundary

The terminal module does not access PS/2, USB, framebuffer, interrupt, or driver
structures. A platform adapter supplies one input callback and the shell uses an
output callback.

```text
Member 11 / platform terminal adapter
              |
              v
       terminal read callback
              |
              v
    Userspace terminal loop (M9)
              |
              v
        Xyris shell (M9)
              |
              v
       SDK / ABI (M10)
```

## Input semantics

- A positive read result supplies one character.
- Zero indicates end-of-input.
- A negative result indicates an input failure.
- Backspace and DEL remove one buffered character.
- Newline submits the current line.
- Control characters other than newline and backspace are ignored.

## Limits

The line buffer uses `XYRIS_SHELL_LINE_MAX` and never writes past its bound.
Long lines are rejected and the current input is discarded.

## Scope boundary

The module does not add or change System ABI syscalls. Keyboard and terminal
device access remains outside this userspace module and must be supplied by the
platform/driver layer.
