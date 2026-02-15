# Resolution Study: [Feature/Fix Name]

**Issue:** #[issue-number] — [Issue Title]
**Agent(s):** [Agent ID(s) that worked on this]
**Wave(s):** [Wave number(s)]
**Date of Study:** [YYYY-MM-DD]
**Final PR:** #[pr-number-that-merged]

**Tags:** [Select all that apply: build, test, architecture, git, api, hardware, cli, docs, process, context]

---

## Summary

[2-3 sentences: What was the issue? Who attempted it? What was the final resolution?]

---

## Original Issue

### Problem Statement
[What needed to be built or fixed?]

### Acceptance Criteria
[What would "done" look like?]

### Constraints
[Any specific requirements, technical constraints, or scope boundaries]

---

## My Original Approach

### Strategy
[What was your high-level plan?]

### Implementation
[What did you actually implement?]

**Key files I modified:**
- `path/to/file1.cpp` — [What I changed]
- `path/to/file2.hpp` — [What I added]

**Code snippet (my version):**
```cpp
// Show a representative example of your implementation
// Pick the most illustrative piece that shows your approach
```

### Reasoning
[Why did you think this would work? What assumptions did you make?]

---

## What Happened

### Outcome
[Was it rejected? Required revision? Partially merged?]

### Feedback Received
[What did the reviewer or orchestrator say?]

### Problems Identified
[What specifically was wrong with your approach?]

---

## Final Merged Solution

### Strategy (Final)
[What was the actual strategy that got merged?]

### Implementation (Final)
[What was actually implemented in the final PR?]

**Key files in final solution:**
- `path/to/file1.cpp` — [What changed in final version]
- `path/to/file2.hpp` — [What was different]

**Code snippet (final version):**
```cpp
// Show the corresponding code from the merged solution
// Same section as your version for direct comparison
```

### Author's Reasoning
[If available from PR description or commit messages: why was this approach chosen?]

---

## Delta Analysis

### High-Level Differences

| Aspect | My Approach | Final Approach | Impact |
|--------|------------|----------------|---------|
| Architecture | [My design] | [Final design] | [Why it matters] |
| File structure | [My layout] | [Final layout] | [Why it matters] |
| Test strategy | [My tests] | [Final tests] | [Why it matters] |
| Scope | [What I included] | [What was included] | [Why it matters] |

### Detailed Code Comparison

**Section 1: [Component/Feature Name]**

*My version:*
```cpp
// Your implementation
```

*Final version:*
```cpp
// Merged implementation
```

*Key difference:* [What's fundamentally different? Not just syntax, but approach]

*Why it matters:* [What makes the final version better/correct?]

---

**Section 2: [Another Component/Feature]**

[Repeat the comparison structure]

---

### What I Missed

1. **[Specific thing I overlooked]**
   - How it manifested: [Where this showed up in your code]
   - Why I missed it: [Root cause]
   - How to catch it next time: [Concrete check to add]

2. **[Another thing I missed]**
   - [Same structure]

3. **[Pattern I didn't recognize]**
   - [Same structure]

---

## Root Cause Analysis

### Why My Approach Didn't Work

[Go deeper than "I made a mistake" — what was the fundamental misunderstanding?]

```
Example:
- I assumed the driver pattern meant X, but it actually means Y
- I treated State A and State B as similar when they have fundamentally different lifecycle requirements
- I didn't understand the relationship between Device and PDN classes
- I thought "add tests" meant unit tests, but integration tests were needed
```

### Faulty Assumptions

| I Assumed... | Reality Was... | How to Verify Next Time |
|--------------|----------------|-------------------------|
| [What you thought was true] | [What's actually true] | [How to check this] |
| [Another assumption] | [Actual truth] | [Verification method] |

---

## Extracted Patterns

### Pattern 1: [Name of Pattern]

**When to apply:** [Conditions/context where this pattern is relevant]

**How to apply:**
1. [Step-by-step guide]
2. [Second step]
3. [Third step]

**Example:**
```cpp
// Concrete code example showing the pattern
```

**Verification:**
```bash
# Command to verify you applied it correctly
```

---

### Pattern 2: [Name of Another Pattern]

[Same structure as Pattern 1]

---

### Pattern 3: [Name of Third Pattern]

[Same structure]

---

## Lessons for Future Work

### Before Starting Implementation

- [ ] [Specific thing to check before coding]
- [ ] [Another pre-implementation verification]
- [ ] [Third checkpoint]

### During Implementation

- [ ] [Thing to verify while writing code]
- [ ] [Another mid-implementation check]
- [ ] [Third validation step]

### Before Submitting PR

- [ ] [Pre-submission verification]
- [ ] [Another final check]
- [ ] [Last sanity test]

---

## Generalized Principles

[What broader principles can you extract that apply beyond this specific issue?]

1. **[Principle Name]**
   - Explanation: [What this principle means]
   - Application: [How to apply it in practice]
   - Example: [Concrete scenario where it matters]

2. **[Another Principle]**
   - [Same structure]

---

## Related Issues

[Link to other issues where these patterns or lessons apply]

- #[issue] — [How it relates]
- #[issue] — [How it relates]
- #[issue] — [How it relates]

---

## Verification of Understanding

### Self-Test

To verify I've internalized these lessons, I should be able to:

1. **[Specific capability]**
   - Test: [How to verify this]
   - Success criteria: [What correct looks like]

2. **[Another capability]**
   - [Same structure]

### Practice Exercise

[Suggest a small, related task you could do to practice the pattern]

```
Example:
"Find another state in the game/ directory and trace its lifecycle.
Verify it matches the pattern I learned from this resolution study."
```

---

## Files to Study

[List of files that exemplify the correct patterns]

- `path/to/example1.cpp` — [Why this is a good example]
- `path/to/example2.hpp` — [What pattern it demonstrates]
- `path/to/test.cpp` — [How tests should be structured]

### Recommended Reading Order

1. [First file to read] — [What to look for]
2. [Second file] — [What to extract]
3. [Third file] — [What pattern to notice]

---

## References

- **GitHub Issue:** #[issue-number]
- **My PR(s):** #[pr-numbers]
- **Final Merged PR:** #[pr-number]
- **Related Docs:** [CLAUDE.md sections, architecture docs, etc.]
- **Related Post-Mortem:** [Link if you wrote one]

---

## Reflection

### What Surprised Me

[What did you discover that was unexpected?]

### What I'd Do Differently

[If you could redo this task, what would you change?]

### Open Questions

[Are there still things you don't fully understand?]

---

## Meta-Learning

### About My Process

[What did this study teach you about your own approach to tasks?]

### About Context Management

[Did context limits play a role? How did that affect your work?]

### About Pattern Recognition

[What made it hard to recognize the correct pattern initially?]

---

*Template version: 1.0*
*Created: 2026-02-15*
