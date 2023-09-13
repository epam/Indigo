# General Info

The frontend is a React app, which was reworked from the legacy Mithrill library. It features two primary
paths: `/search` and `/libs`.

## /search

The `/search` path includes a Ketcher window and various tools for searching molecules from libraries. As a user, you
can:

- Search using Bingo-Elastic
- Search using Postgres
- Search by different match types (exact match, similarity match, submatch)
- Draw the desired molecule using the Ketcher window

## /libs

The `/libs` path requires a password for user access. The password is stored in the `REACT_APP_LIBS_PASSWORD`
environment variable. In the libs tab, users can:

- Add new libraries
- Remove a library
- Upload .sdf files to a library

By exploring the `/search` and `/libs` paths, users can access the essential features of the Indigo toolkit through the
Indigo Service.

## How to build

To build react app use next command:

```
yarn run build
```
