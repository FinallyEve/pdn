# Agent Feedback Loop System

## Overview

The agent feedback loop system is a structured mechanism for AI agents to learn from past mistakes, successful resolutions, and patterns discovered during development. This system enables continuous improvement across the agent fleet by capturing learnings and making them accessible for future reference.

## Purpose

**Why this system exists:**
- Agents hit context limits and lose detailed memory of earlier work
- Similar mistakes are repeated across waves when learnings aren't captured
- Successful resolution patterns need to be extracted and shared
- Post-mortems help the orchestrator identify systemic issues

**What it provides:**
- Structured templates for documenting what went wrong and why
- Resolution study templates for comparing attempted vs. successful approaches
- A searchable repository of learnings categorized by topic
- Evidence for the orchestrator that agents participated in reflection

## How to Use This System

### For Junior Dev Agents (Post-Mortems)

When your PR is rejected, merged with significant changes, or requires multiple revision cycles:

1. **Create a post-mortem** in `learnings/` using the template:
   ```bash
   cp templates/post-mortem.md learnings/wave-XX-issue-YY-post-mortem.md
   ```

2. **Fill it out completely** — don't skip sections:
   - What you tried and why
   - What went wrong (root cause, not symptoms)
   - What the final resolution was
   - Concrete lessons learned for future work

3. **Self-tag the issue** by adding your analysis as a comment on the original GitHub issue

4. **Commit the post-mortem** to your branch before the PR closes (or immediately after if merged)

### For All Agents (Resolution Studies)

When you return to study how an issue you worked on was ultimately resolved:

1. **Create a resolution study** in `learnings/` using the template:
   ```bash
   cp templates/resolution-study.md learnings/wave-XX-issue-YY-resolution-study.md
   ```

2. **Analyze the delta** between your approach and the final merged solution:
   - What was fundamentally different?
   - What assumptions did you make that turned out to be wrong?
   - What pattern can you extract for future use?

3. **Extract actionable patterns** — not vague advice like "read more carefully" but concrete rules like:
   - "Always check if a test registration file already exists before creating a new one"
   - "When adding minigame tests, grep for existing TEST_F patterns first"
   - "Verify file structure matches CLAUDE.md before implementing"

### For the Orchestrator

**Verification checklist:**
- [ ] Post-mortem exists in `docs/agent-learnings/learnings/` for rejected PRs
- [ ] Agent added a self-reflection comment to the GitHub issue
- [ ] Resolution study exists for issues that required multiple attempts
- [ ] Learnings are tagged with relevant categories (build, test, architecture, git, etc.)

**When to require post-mortems:**
- PR rejected due to fundamental misunderstanding
- Agent required 3+ revision cycles
- Agent created duplicate files or tests
- Agent ignored existing patterns documented in CLAUDE.md
- Agent made changes outside assigned scope

## Directory Structure

```
docs/agent-learnings/
  README.md                           — This file
  templates/
    post-mortem.md                    — Template for agent post-mortems
    resolution-study.md               — Template for studying final resolutions
  learnings/
    wave-XX-issue-YY-post-mortem.md   — Completed post-mortem documents
    wave-XX-issue-YY-resolution-study.md — Resolution analysis documents
```

## Naming Conventions

**Post-mortems:**
```
wave-{wave-number}-issue-{issue-number}-post-mortem.md
```
Example: `wave-15-issue-42-post-mortem.md`

**Resolution studies:**
```
wave-{wave-number}-issue-{issue-number}-resolution-study.md
```
Example: `wave-15-issue-42-resolution-study.md`

**If multiple agents worked on the same issue, add agent ID:**
```
wave-15-issue-42-agent-03-post-mortem.md
wave-15-issue-42-agent-07-post-mortem.md
```

## Tags for Categorization

Use these tags in your post-mortems and resolution studies to make learnings searchable:

- `build` — Build system, PlatformIO, compilation issues
- `test` — Test writing, test registration, test environments
- `architecture` — State machines, drivers, game structure
- `git` — Branch management, commits, PR workflow
- `api` — API design, message types, protocol
- `hardware` — ESP32-specific, driver implementations
- `cli` — CLI simulator, native builds
- `docs` — Documentation, CLAUDE.md, plan docs
- `process` — Workflow, checkpoints, phase gates
- `context` — Context limit issues, state recovery

## Integration with Agent Workflow

This system complements the existing agent workflow defined in `~/Documents/code/alleycat/plans/pdn/workflow.md`:

1. **Phase 3 (Junior Dev)** — If your PR is rejected, write a post-mortem before closing the loop
2. **Between waves** — Senior agents review learnings from the previous wave
3. **Retrospectives** — Orchestrator uses post-mortems to identify systemic patterns
4. **Onboarding** — New agents read recent post-mortems as part of orientation

## Best Practices

**For post-mortems:**
- Write them immediately while context is fresh
- Be honest about what you misunderstood — this helps future agents
- Include code snippets showing before/after
- Link to relevant files, commits, and PR comments

**For resolution studies:**
- Wait until the issue is fully resolved and merged
- Compare your approach against the final merged code, not intermediate attempts
- Focus on extracting patterns, not assigning blame
- Test your extracted patterns against other similar code in the repo

**For all learnings:**
- Use concrete examples, not abstract advice
- Make them searchable — future agents will grep for keywords
- Update them if you discover your analysis was wrong
- Don't duplicate content already in CLAUDE.md — reference it instead

## Examples

See the templates in `templates/` for detailed examples of:
- A complete post-mortem with root cause analysis
- A resolution study with delta comparison
- Proper tagging and categorization
- Actionable lesson extraction

---

*This system was created in Wave 15 to address repeated context loss and pattern-matching failures across the agent fleet.*
