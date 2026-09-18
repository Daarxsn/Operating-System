# XyrisOS Userspace Shell Contract — M9.5

## Scope

The shell owns command parsing and command dispatch in userspace.

The shell does not own keyboard hardware, terminal drivers, or syscall/ABI definitions.

Input is supplied as one complete command line.
Output is supplied through a writer callback.

This separation allows a future terminal service to attach interactive input/output without changing the shell parser or commands.

## V1 commands

- `help`
- `echo <text...>`
- `services`
- `service <name>`
- `system`
- `quit`

## Parsing

- Maximum line length: 127 characters excluding the terminator.
- Maximum arguments: 16.
- Maximum token length: 63 characters excluding the terminator.
- Whitespace separates arguments.
- Single and double quotes group text into one argument.

## Service access

The shell uses the userspace service registry.
It does not access kernel structures or service implementation internals directly.

## Terminal dependency

Interactive keyboard input and terminal output are not part of M9.5.
The shell API is therefore transport-neutral.
