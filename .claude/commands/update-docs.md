# Update Documentation for Topic

Your task is to check whether the project documentation is accurate and
up-to-date for a **specific topic**, then make targeted updates only where
needed. Do not refactor or reorganize anything outside the scope of this topic.

The topic is: **$ARGUMENTS**

---

## Step 1 — Follow the Reference Chain

Do not explore the codebase freely. Navigate only through what the docs point to.

1. Read `claude.md` — find any section, line, or reference mentioning `$ARGUMENTS`
2. If a reference exists, read only the sub-document it points to (e.g. `.claude/claude-docs/testing.md`)
3. From that sub-document, read only the source files explicitly named in it

If `$ARGUMENTS` has no reference in `claude.md` at all:
- Grep for `$ARGUMENTS` keywords inside `.claude/claude-docs/` only — not the whole codebase
- If a match is found in a sub-document, continue from step 2
- If no match is found anywhere in `.claude/claude-docs/`, note that this topic is undocumented
  and skip to Step 3

Stop following the chain the moment you have enough context to compare
docs against reality. Do not read files out of curiosity.

---

## Step 2 — Identify Gaps

Compare what you found in the source files against what the docs say.
Look only for issues directly related to `$ARGUMENTS`:

- **Missing** — something exists in code but is not mentioned in docs
- **Outdated** — docs describe old behavior, removed features, renamed files,
  or changed conventions
- **Wrong location** — the information exists but is in the wrong file or section
- **Duplicated** — the same detail appears in both `claude.md` and a sub-document

If everything is accurate and complete, say so and stop. Do not edit anything.

---

## Step 3 — Plan Before You Edit

Before changing any file, output a plan in this format:


Topic: [topic from $ARGUMENTS]
Status: [up to date | gaps found | undocumented]
Reference chain followed: claude.md → .claude/claude-docs/[name].md → [source files read]
Changes planned:

[file]: [what will change and why]
[file]: [what will change and why]


If the status is "up to date", stop here.

If a planned change would delete or significantly rewrite existing content,
describe the tradeoff explicitly before proceeding.

---

## Step 4 — Make Targeted Edits

Apply only the changes identified in Step 2. Follow these rules strictly:

- Edit only sections directly related to `$ARGUMENTS`
- Do not reformat, reorder, or rewrite sections that are already correct
- Detail belongs in sub-documents, not in `claude.md` — if new content is
  substantial, add it to the relevant `.claude/claude-docs/` file and update or add the
  reference line in `claude.md`
- If no sub-document exists for this topic yet, create `.claude/claude-docs/[topic].md`
  and add a reference in `claude.md` using this format:

  `See [.claude/claude-docs/filename.md](.claude/claude-docs/filename.md) for [short description].`

- Any new content added directly to `claude.md` must be 3 lines maximum

---

## Step 5 — Validate

After editing, confirm:
- Every link you added or touched resolves to a real file on disk
- You did not read or modify anything outside what the reference chain led to
- `claude.md` is the same length or shorter than before (detail goes in sub-docs)

---

## Output

Finish with a concise summary:


Topic: [topic from $ARGUMENTS]
Chain: claude.md → .claude/claude-docs/[name].md → [source files]
─────────────────────────────────────────────────────
✅ Everything up to date — no changes made
OR
📝 .claude/claude-docs/[name].md — updated [section]: [one line description]
📄 .claude/claude-docs/[name].md — created: [one line description of content]
📝 claude.md — updated reference: [one line description]