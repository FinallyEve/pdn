# Post-Mortem: [Brief Description of What Went Wrong]

**Issue:** #[issue-number] — [Issue Title]
**Agent:** [Agent ID, e.g., claude-agent-03]
**Wave:** [Wave number, e.g., Wave 15]
**Date:** [YYYY-MM-DD]
**PR:** #[pr-number] (if applicable)
**Status:** [Rejected / Merged with significant changes / Required N revision cycles]

**Tags:** [Select all that apply: build, test, architecture, git, api, hardware, cli, docs, process, context]

---

## Summary

[2-3 sentences summarizing what you were trying to do, what went wrong, and what the actual resolution was]

---

## What I Tried

### Initial Understanding
[What did you think the task required? What was your mental model?]

### Approach Taken
[Step-by-step breakdown of what you actually did]

```
Example:
1. Read CLAUDE.md section on test registration
2. Created new test file: test/test_cli/my-new-tests.cpp
3. Added TEST_F macros directly in the .cpp file
4. Ran `pio test -e native_cli_test`
5. Tests passed locally
6. Committed and pushed
```

### Code Examples

**Before (what I wrote):**
```cpp
// Include the actual code you wrote that caused the problem
// Be specific — show the exact mistake
```

**What I expected to happen:**
[Describe your expectation]

---

## What Went Wrong

### Root Cause
[Not "I made a mistake" — dig deeper. Why did you make that mistake? What was the underlying misunderstanding?]

```
Example root causes:
- Misread CLAUDE.md: thought test registration went in .cpp, missed that TEST_F goes in separate registration files
- Assumed CLI test pattern matched core test pattern (test_core/ vs test_cli/)
- Didn't grep for existing files before creating new ones
- Confused build environment (native_cli) with test environment (native_cli_test)
```

### Contributing Factors
[What made this mistake more likely?]

```
Example:
- Context was compacted before I read the test registration section of CLAUDE.md
- Similar-looking patterns in test_core/ led me astray
- Test passed locally, giving false confidence
- Didn't verify against existing test files in the same domain
```

### Symptoms vs. Cause
[What were the visible symptoms? What was the actual underlying cause?]

---

## What Actually Fixed It

### The Correct Approach

**After (correct solution):**
```cpp
// Show the correct code or approach
// Highlight what's different from your attempt
```

### Key Differences

| What I Did | What Should Have Been Done | Why It Matters |
|-----------|---------------------------|---------------|
| Created `test/test_cli/my-tests.cpp` with TEST_F macros | Should have split into `my-tests.hpp` (functions) + `my-reg-tests.cpp` (TEST_F) | CLI tests use split pattern: headers for logic, cpp for registration |
| Added tests to new file | Should have checked if registration file already existed | Would have found existing `my-reg-tests.cpp` from earlier work |
| Assumed pattern from test_core/ | Should have read CLAUDE.md section specific to CLI tests | Different test environments have different conventions |

---

## Lessons Learned

### Concrete Patterns to Apply

1. **[Specific rule extracted from this experience]**
   - Example: "Always grep for existing test registration files before creating new ones: `grep -r 'TEST_F.*MyDomain' test/test_cli/`"

2. **[Another specific, actionable pattern]**
   - Example: "CLI tests split logic (.hpp) from registration (.cpp). Check table in CLAUDE.md 'Test File Convention' section"

3. **[What to check before doing this type of task again]**
   - Example: "Before adding tests, verify: (1) which domain, (2) does reg file exist, (3) read existing tests in that domain first"

### Questions to Ask Next Time

- [ ] [Specific question relevant to this type of task]
- [ ] [Another verification step to add to your checklist]
- [ ] [A way to catch this mistake earlier in the process]

### What to Grep For

```bash
# Commands that would have caught this mistake early
grep -r "TEST_F.*MyDomain" test/test_cli/
ls test/test_cli/*reg-tests.cpp
git log --oneline --follow test/test_cli/my-domain-tests.hpp
```

---

## File Reference

### Files I Modified (Incorrectly)
- `path/to/file.cpp` — [What I changed and why it was wrong]

### Files I Should Have Modified
- `path/to/correct/file.cpp` — [What should have been changed]

### Files I Should Have Read First
- `CLAUDE.md` lines XXX-YYY — [Specific section I missed]
- `test/test_cli/existing-example.cpp` — [Example that shows the right pattern]

---

## Verification

### How to Verify This Pattern

[Concrete steps to test whether you've internalized this lesson]

```bash
# Example verification:
# 1. Find a similar task
# 2. Apply the lesson learned
# 3. Check if the result matches existing patterns
```

### Self-Check Questions

Before doing a similar task again, ask:
- [ ] [Specific question from this experience]
- [ ] [Another verification checkpoint]
- [ ] [Final sanity check]

---

## References

- **GitHub Issue:** #[issue-number]
- **Pull Request:** #[pr-number]
- **Related PRs:** #[other-relevant-prs]
- **CLAUDE.md Section:** [Specific section that explains the correct approach]
- **Related Post-Mortems:** [Link to other learnings on similar topics]

---

## Notes

[Any additional context, observations, or things you discovered while writing this post-mortem]

---

*Template version: 1.0*
*Created: 2026-02-15*
