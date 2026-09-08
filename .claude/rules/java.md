---
paths:
  - "api/java/**/*.java"
  - "bingo/bingo-elastic/java/**/*.java"
  - "**/pom.xml"
description: "Java standards for the Indigo JNA wrappers and the Bingo Elastic Java client"
---

# Java rules

## Baseline

- Source and target level: Java 8 — the wrappers ship to consumers who have not moved on.
- Maven via the wrapper (`mvnw`); versions flow from `${revision}` through `flatten-maven-plugin`,
  so never hard-code a version in a module POM.
- Modules under `api/java/`: `indigo`, `indigo-inchi`, `indigo-renderer`, `bingo-nosql`.

## JNA boundary

- Native functions are declared once, in the `IndigoLib` interface; loading goes through
  `Native.load("indigo", IndigoLib.class)` with platform-aware resolution.
- A native return of `-1` becomes an `IndigoException` (unchecked) carrying the message from
  `indigoGetLastError`. Never surface the raw sentinel.
- Native handles are released deterministically — `try`-with-resources or an explicit `dispose()`.
  Finalizers are not a resource strategy.

## Conventions

- `PascalCase` types, `camelCase` methods, `UPPER_SNAKE_CASE` constants, packages under
  `com.epam.indigo`.
- Javadoc states the contract: `@param`, `@return`, `@throws`. It does not narrate the implementation.
- New wrapper methods follow the shape of the existing `IndigoObject` methods. Consistency with the
  surrounding wrapper beats local elegance — a wrapper that is half in one style is harder to use
  than one that is uniformly imperfect.

## Tests

- JUnit 5; Testcontainers for anything that needs a live Elasticsearch.
- Every public method has at least one test, and every documented failure mode has a negative test
  asserting the exception type and message.

## Anti-patterns

- `System.out.println` as logging; empty `catch` blocks; wildcard imports; raw collection types.
