# XyrisOS ABI Compatibility Validation

This test suite verifies the public ABI compatibility contract for M10 7.9.

It covers:

- ABI version extraction and major-version matching;
- minor-version requirement checks;
- fixed-width ABI scalar sizes;
- extensible ABI header size and field offsets;
- uniqueness of syscall numbers;
- presence of the normative 7.9 compatibility policy;
- `.xapp` ABI declarations when package artifacts are present.

The tests intentionally validate the public contract without depending on private kernel structures.
