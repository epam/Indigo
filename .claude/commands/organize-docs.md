# Organize Project Documentation

Your task is to analyze the project, then organize all documentation into a
clean, token-efficient structure using a main `claude.md` file with references
to focused sub-documents.

---

## Step 1 — Audit Existing Content

1. Read the current `claude.md` (or `CLAUDE.md`) if it exists.
2. Scan the project for any existing `.md` files containing documentation,
   instructions, or context relevant to Claude (architecture, API design,
   DB schema, deployment, conventions, etc.).
3. Identify sections in `claude.md` that are long, detailed, or self-contained
   enough to live in their own file.

---

## Step 2 — Decide What Stays vs. What Gets Extracted

**Keep inline in `claude.md`:**
- One-sentence project description
- Stack/tech summary (2–5 lines max)
- Critical gotchas or non-obvious rules that apply globally
- The full reference table (see Step 4)

**Extract to a sub-document when a section:**
- Exceeds ~20 lines, OR
- Covers a single distinct topic (architecture, API, DB, auth, deployment…), OR
- Would not be relevant in every Claude conversation

---

## Step 3 — Write the Sub-Documents

For each extracted topic, create or update a file under `.claude/claude-docs/`
(e.g. `.claude/claude-docs/architecture.md`, `.claude/claude-docs/api.md`, `.claude/claude-docs/database.md`).

Each sub-document must:
- Start with a single `#` heading matching the topic name used in the reference
- Be fully self-contained — do not assume the reader has read other docs
- Use clear headings, code blocks, and examples where useful
- End with a `## Related` section linking to sibling docs

---

## Step 4 — Rewrite `claude.md`

Produce a clean, minimal `claude.md` using this exact structure:

```markdown
# [Project Name]

[One paragraph: what this project does, tech stack, and the single most
important thing to know before touching the code.]

## Quick Reference

| Topic        | File                                            | What's inside                        |
|--------------|-------------------------------------------------|--------------------------------------|
| Architecture | [.claude/claude-docs/architecture.md](.claude/claude-docs/architecture.md)   | System design, components, data flow |
| API          | [.claude/claude-docs/api.md](.claude/claude-docs/api.md)                     | Endpoints, auth, request/response    |
| Database     | [.claude/claude-docs/database.md](.claude/claude-docs/database.md)           | Schema, migrations, query patterns   |
| Deployment   | [.claude/claude-docs/deployment.md](.claude/claude-docs/deployment.md)       | Environments, CI/CD, secrets         |
| Conventions  | [.claude/claude-docs/conventions.md](.claude/claude-docs/conventions.md)     | Code style, naming, patterns         |

## Key Rules

- [Rule 1 — only things that apply in every context]
- [Rule 2]
- [Rule 3]

## See Also

- See [.claude/claude-docs/architecture.md](.claude/claude-docs/architecture.md) for full system design.
- See [.claude/claude-docs/api.md](.claude/claude-docs/api.md) for endpoint contracts.
- See [.claude/claude-docs/conventions.md](.claude/claude-docs/conventions.md) before writing any new code.
\```

Only include rows/links for docs that actually exist.
Do not add placeholder rows for topics not yet documented.

---

## Step 5 — Validate

Before finishing, confirm all of the following:
- Every link in `claude.md` resolves to a real file on disk
- No section in `claude.md` exceeds 30 lines (excluding the Quick Reference table)
- Each sub-document is self-contained and starts with a `#` heading
- No content is duplicated between `claude.md` and sub-documents
- `claude.md` still provides a useful mental model on its own

---

## Output

When done, print a summary:

```
✅ claude.md updated (~XX lines, was ~XX lines)
📄 Created: .claude/claude-docs/architecture.md
📄 Created: .claude/claude-docs/api.md
📝 Updated: .claude/claude-docs/conventions.md