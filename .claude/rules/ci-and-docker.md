---
paths:
  - ".github/workflows/**"
  - ".ci/**"
  - "**/Dockerfile*"
  - "**/docker-compose*.y*ml"
description: "CI/CD and container rules: GitHub Actions workflows, Docker images, release artifacts"
---

# CI and Docker rules

## GitHub Actions

- Every workflow declares a `concurrency` group with cancel-in-progress; without it a busy day of
  pushes queues hours of duplicated matrix builds.
- Actions are pinned to a released major (`actions/checkout@v4`), never to `@main`.
- Credentials come from `${{ secrets.* }}`. A literal token in a workflow is an incident, not a bug.
- The platform matrix that matters here is Linux x86_64 and aarch64, Windows x64, macOS x86_64 and
  arm64 — a change that builds on one leg is not verified.
- Cache pip, Maven and ccache with keys derived from lock files, so a dependency bump invalidates
  the cache instead of poisoning it.

## Dockerfiles

- Base images must be supported distributions. The glibc of the build image must match the runtime
  image, or the artifact fails at load time with a missing-symbol error that looks nothing like the
  real cause.
- Multi-stage: a build/test stage, then a minimal runtime stage that copies only artifacts.
- Run as a non-root user, add a `HEALTHCHECK` for services, pass `--no-cache-dir` to pip, and keep
  `.dockerignore` accurate — the checkout is large enough that context size decides build time.

## Compose

- Health checks per service; resource limits for anything production-shaped; configuration through
  an `.env` file rather than literals in the YAML.

## Releases

- A tag `indigo-X.Y.Z` triggers the full matrix and publication to PyPI, Maven Central, NuGet, npm
  and Conda. The version is derived from the tag by `api/indigo-version.cmake` — it is not edited by
  hand.
- Update the changelog before pushing the tag, not after.
