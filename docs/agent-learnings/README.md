# Agent Feedback Loop System

## Purpose

This directory contains the infrastructure for the agent fleet's structured feedback loop system. The goal is to enable agents to learn from past mistakes, study successful resolutions, and build a knowledge base of patterns and anti-patterns.

## Why This Exists

AI agents working on complex codebases will make mistakes. The key to improvement is not avoiding mistakes entirely, but:

1. **Documenting failures** so the same mistake isn't repeated
2. **Analyzing successes** to understand what patterns work
3. **Building a knowledge base** that future agents (and humans) can reference
4. **Creating accountability** through structured reflection

## Directory Structure

```
docs/agent-learnings/
├── README.md                           # This file
├── templates/
│   ├── post-mortem.md                  # Template for agent post-mortems
│   └── resolution-study.md             # Template for studying resolutions
└── learnings/
    └── [agent-generated learning files]
```

## How Agents Should Use This System

### 1. After a Failed Task or Blocked Issue

When an agent encounters a significant failure, blocking issue, or makes a mistake that causes wasted effort:

1. Copy `templates/post-mortem.md` to `learnings/issue-XXX-post-mortem.md`
2. Fill out the template with:
   - Root cause analysis
   - What was tried and why it failed
   - What actually fixed it (if known)
   - Lessons learned and patterns to apply/avoid
3. Commit the post-mortem to the branch
4. Reference it in the PR description or issue comments

**When to write a post-mortem:**
- Task took significantly longer than expected
- Multiple approaches were tried before finding the solution
- A build broke, tests failed unexpectedly, or CI blocked the PR
- The agent misunderstood requirements or project conventions
- A rollback or significant rework was required

### 2. After an Issue is Resolved (Resolution Study)

When an issue that an agent worked on is finally merged (by the same agent, a different agent, or a human):

1. Copy `templates/resolution-study.md` to `learnings/issue-XXX-resolution-study.md`
2. Compare the original agent's approach with the final merged solution
3. Perform a delta analysis: what was different and why it mattered
4. Extract a reusable pattern for future tasks
5. Commit the resolution study

**When to write a resolution study:**
- An agent's PR was closed and a different approach was merged
- An agent got partway through a task and another agent finished it
- Human review resulted in significant changes to the approach
- The agent wants to document a successful pattern for future reference

### 3. Before Starting a Similar Task

When an agent is assigned a new task:

1. **Search this directory** for related learning files using keywords from the issue
2. Read any relevant post-mortems or resolution studies
3. Apply the lessons learned and avoid documented anti-patterns
4. If the task involves similar code/systems, reference the relevant learning docs

**How to search learnings:**
```bash
# Search for keyword in all learning files
grep -r "keyword" docs/agent-learnings/learnings/

# List all learnings
ls -la docs/agent-learnings/learnings/
```

## How the Orchestrator Uses This System

The orchestrator verifies agent participation in the feedback loop by:

1. **Checking for post-mortems** when tasks exceed estimated time or encounter failures
2. **Requiring resolution studies** when an agent's PR is closed/superseded
3. **Auditing learning application** by checking if agents reference existing learnings before starting tasks
4. **Measuring improvement** by tracking whether similar mistakes decrease over time

## Guidelines for Writing Effective Learnings

### Do:
- **Be specific**: Include file names, function names, exact error messages
- **Include code snippets**: Show before/after comparisons
- **Explain the "why"**: Don't just document what happened, explain why it matters
- **Tag appropriately**: Use the tags section so learnings are discoverable
- **Link to related issues/PRs**: Provide context for future readers

### Don't:
- **Don't blame or complain**: Focus on learning, not fault
- **Don't be vague**: "The tests failed" is not useful; "Test X failed because Y" is
- **Don't skip failed approaches**: Document what didn't work and why
- **Don't write novels**: Be thorough but concise
- **Don't forget to tag**: Untagged learnings are hard to find

## Integration with Workflow

This system integrates with the agent workflow defined in `~/Documents/code/alleycat/plans/pdn/workflow.md`:

- **Junior Dev** writes post-mortems when blocked or making mistakes
- **Testing Agent** writes post-mortems when tests reveal unexpected issues
- **Senior Dev** writes resolution studies when reviewing other agents' work
- **Orchestrator** enforces post-mortem creation when tasks fail or exceed estimates

## Examples of Good Learnings

### Example Post-Mortem Topics:
- "Why `pio test -e native_cli` doesn't work" (environment confusion)
- "Git force-push to main blocked by hooks" (workflow violation)
- "Tests pass locally but fail in CI" (environment differences)
- "Forgot to run tests before committing" (process failure)

### Example Resolution Study Topics:
- "How FDN integration tests should be structured" (pattern extraction)
- "The correct way to add new CLI commands" (convention documentation)
- "Why split-file pattern is used for test registration" (architecture understanding)
- "How to handle ESP-NOW message size limits" (technical constraint)

## Maintenance

This directory should be:
- **Reviewed regularly** by the orchestrator for patterns
- **Cleaned up** if learnings become outdated (e.g., after major refactors)
- **Referenced in CLAUDE.md** when patterns solidify into conventions
- **Used to update workflow.md** when process improvements are identified

## Questions?

If you're unsure whether to write a post-mortem or resolution study, **err on the side of documenting it**. It's better to have too much learning material than too little.

---

*Created: 2026-02-15*
*Part of Wave 15 agent fleet infrastructure improvements*
