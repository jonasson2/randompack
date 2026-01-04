# Contributing to Randompack

Thank you for your interest in contributing to Randompack.

## Scope

Randompack prioritizes correctness, reproducibility, and clarity over feature
count. Contributions are welcome if they align with these goals.

In particular:
- Bug fixes and correctness improvements are always welcome.
- Performance improvements are welcome when they preserve correctness and
  reproducibility.
- New random number generators or distributions should be based on published,
  well-documented algorithms.

## Coding standards

- The core library is written in ISO C11.
- Code should be portable and avoid undefined behavior.
- Changes should not break reproducibility of existing generators or
  distributions.
- Public API changes should be discussed before submission.

## Testing

All contributions should include appropriate tests.
Existing tests must continue to pass.

## Submitting changes

Please submit changes via pull requests. Include a clear description of the
problem being addressed and the rationale for the proposed solution.

---
