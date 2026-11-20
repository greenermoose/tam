# Lua scripting topic in the ecosystem chapters (tam 0.15.0)

- **Date:** 2026-10-14
- **Stage:** Develop — a new corpus topic,
  `knowledge/ecosystem/lua.html`, was added to the offline guide, with
  `explain`/`search` coverage and refreshed index/snapshots. No version
  change (tam stays 0.15.0).
- **CLI Tool:** `opencode` 1.19.0
- **Model:** `big-pickle`
- **Transcript:** Retained privately by the author.
- **Commit:** `a6a97bb`
- **Authorship:** Fred directed the work and supplied a draft embedded-Lua
  developer guide as the source material, asking opencode to check its
  veracity, rewrite it in house style, and ground it in this system.
  opencode verified the claims against the installed packages and upstream
  Hyprland sources, wrote the page and its tests, and committed the change.

## Guiding prompts

> Work on the tam knowledge corpus. Here's some info related to lua that I
> want to use for the basis of tam explain lua. Double check the veracity
> and consider rewriting to follow our style guides. When you are done, tam
> explain lua should give the user a good understanding of lua on this
> system. Be sure to include why we include lua.
>
> While you're doing that, I'm going to ask agy to work on the next
> milestone for the tam command. I'm hoping the two of you can work in
> parallel without too much conflict.

The same message pasted a developer guide, "Developer Guide: Scripting with
Embedded Lua", as the source material to check. It is reproduced verbatim:

```
## Developer Guide: Scripting with Embedded Lua

Your system includes an embedded **Lua runtime**. Lua allows you to write lightweight scripts to customize application behavior, automate workflows, and exchange structured data with the core system without recompiling native code.

---

### Core Concept: The Shared Stack

The host system (written in C/C++) and your Lua scripts communicate through a shared **virtual stack**:

* To give Lua an input, the host pushes arguments onto the stack.
* When your script finishes, it pushes return values back onto the stack.
* Values can be accessed from the bottom up (`1, 2, 3...`) or from the top down (`-1, -2, -3...`).

```text
[ Stack Top / -1 ]  -->  Returned Table / Latest Value
[ Index 2        ]  -->  Second Argument
[ Index 1        ]  -->  Function or First Argument

```

---

### 1. Writing Your Lua Script

Create a script file or module defining the logic you want the system to run. Lua functions can accept multiple parameters and return structured records using **tables**:

```lua
-- scripts/character.lua

-- Define a function that the host system will trigger
function create_character(name, base_power)
    -- Validate or calculate attributes
    local scaled_power = base_power * 1.5

    -- Return a table containing structured data back to the system
    return {
        name = name,
        power = scaled_power,
        alive = true
    }
end

```

---

### 2. How the Host System Executes Your Script

When the host system calls your script, it performs three primary operations: loading the function, passing parameters, and reading the returned table.

#### Step 1: Push the Function and Arguments

The host locates your function by name and pushes your arguments onto the stack in order:

```c
// 1. Locate the function in Lua's global scope
lua_getglobal(L, "create_character");

// 2. Push arguments left-to-right
lua_pushstring(L, "Valkyrie");  // Argument 1 (name)
lua_pushnumber(L, 120.0);        // Argument 2 (base_power)

```

#### Step 2: Invoke the Protected Call (`lua_pcall`)

The host calls `lua_pcall`, specifying how many arguments were sent and how many return values to expect:

```c
// lua_pcall(state, num_args, num_returns, error_handler_index)
if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
    // If an error occurs, the error message sits on top of the stack (-1)
    const char *err = lua_tostring(L, -1);
    fprintf(stderr, "Script runtime error: %s\n", err);
    lua_pop(L, 1);
    return;
}

```

#### Step 3: Inspect the Returned Table

Once executed, the returned table sits at the top of the stack (`-1`). The host extracts fields using `lua_getfield`:

```c
// Verify the return value is actually a table
if (lua_istable(L, -1)) {
    // Read the "name" field: pushes table["name"] to stack top (-1)
    lua_getfield(L, -1, "name");
    const char *name = lua_tostring(L, -1);
    lua_pop(L, 1); // Discard "name", restoring table to -1

    // Read the "power" field: pushes table["power"] to stack top (-1)
    lua_getfield(L, -1, "power");
    double power = lua_tonumber(L, -1);
    lua_pop(L, 1); // Discard "power"

    // Read the "alive" field: pushes table["alive"] to stack top (-1)
    lua_getfield(L, -1, "alive");
    int is_alive = lua_toboolean(L, -1);
    lua_pop(L, 1); // Discard "alive"

    printf("Loaded: %s | Power: %.1f | Active: %d\n", name, power, is_alive);
}

// Clean up the table from the stack
lua_pop(L, 1);

```

---

### 3. Iterating Dynamic Keys (`lua_next`)

If your script returns a table with arbitrary or dynamic keys (such as an inventory list or key-value map), the host scans it sequentially using `lua_next`:

```c
// Assume the returned table is at index -1
lua_pushnil(L); // Signal lua_next to start from the first element

while (lua_next(L, -2) != 0) {
    // Key is at index -2, Value is at index -1
    const char *key = lua_tostring(L, -2);
    const char *val = lua_tostring(L, -1);

    printf("Attribute: %s => %s\n", key, val);

    // Pop the value, keeping the key on top for the next iteration
    lua_pop(L, 1);
}

```

---

### Best Practices for Script Writers

* **Always Check Return Types:** Ensure your Lua function returns the type the host expects (`table`, `string`, `number`, or `boolean`). If your function returns `nil` unexpectedly, the host will flag a type mismatch.
* **Keep Global Scope Clean:** Use `local` for temporary variables and helper functions inside your script files. Only expose functions meant to be triggered directly by the system as globals.
* **1-Based Indexing:** Remember that standard Lua lists/arrays begin at index `1`, not index `0`.
* **Handle Nil Values:** Missing keys in a Lua table evaluate to `nil` rather than throwing an exception. Ensure required fields are explicitly populated before returning.
```

> Continue if you have next steps, or stop and ask for clarification if you
> are unsure how to proceed.

## Veracity review of the source guide

- **C API claims are correct.** `lua_getglobal`, `lua_pushstring`,
  `lua_pushnumber`, `lua_pcall(L, 2, 1, 0)`, `LUA_OK`, `lua_istable`,
  `lua_getfield`, `lua_tostring`, `lua_tonumber`, `lua_toboolean`, and the
  `lua_next` key-at-`-2`/value-at-`-1` convention with a `lua_pop` per round
  all match the Lua 5.5 manual shipped at
  `/usr/share/doc/lua/manual.html`. 1-based indexing, missing keys reading
  as `nil`, and the `local` guidance are all accurate.
- **Stack diagram corrected.** The source diagram mixed the pre-call state
  (function and arguments) with the post-call state (returned table) in one
  frame. The page describes one state at a time and notes that the call
  consumes its inputs and leaves a single result.
- **"The host will flag a type mismatch" softened.** Whether a wrong or
  missing return is reported depends on the host; the page states the
  contract rather than promising a specific diagnostic.
- **Grounded in this system.** Every claim was tied to evidence here: the
  `lua` 5.5.1 package, `liblua.so.5.5` needed by `/usr/bin/Hyprland` with
  `luaL_loadfilex`/`lua_pcallk`/`lua_next` in its binary, upstream
  Hyprland's `pkg_search_module(LUA REQUIRED ... lua>=5.5 lua<5.6)` and Lua
  `ConfigManager`, the LuaJIT-backed Neovim and mpv packages, and the
  `hyprctl reload`/`repl`/`configerrors` verbs.

## Decisions and scope

- **Page home and level ladder.** The topic lives at
  `knowledge/ecosystem/lua.html` (track `improve`, levels beginner,
  intermediate, expert) and is linked from `ecosystem/index.html` only, as
  the other ecosystem chapters are. It follows the 0.15.0 fading templates:
  a command sequence for beginners, a worked example and a gap-filling
  completion task for intermediates, and a terse C-API reference with
  `man:` links for experts.
- **"Why we include Lua" is answered explicitly** in the beginner section:
  the desktop configuration is Lua (Hyprland embeds it), the editor and
  player embed it too, and it fits the resource-efficiency house rule.
- **No version bump.** The change adds a corpus topic, matching the
  ecosystem-chapters precedent (`a067971`), which did not bump either;
  0.16.0 is reserved for the maintain-track milestone.
- **Coverage added** in `tests/guide.sh` (page list) and `tests/run.sh`
  (`explain lua`, the depth ladder for `lua`, `search lua`), plus a
  regenerated index and the new/updated golden snapshots.
- **One test made topic-precise.** The expert-catalog assertion that
  excluded the beginner-only `desktop` topic matched the bare substring
  `desktop`, which the new Lua summary ("...desktop configuration...") also
  contains; it now anchors to the catalog topic column (`^  desktop `).

## Verification

- `make corpus` regenerated `knowledge/index.{tsv,json}` (26 entries) with
  only the new Lua row added.
- `make` builds and `make test` runs `tests/run.sh` and `tests/guide.sh`
  green, including the golden snapshots (`UPDATE=1` refreshed the ecosystem
  index snapshot for the new entry and created `ecosystem-lua.txt`).
- Manual spot-checks: `tam explain lua` renders the depth line and all
  three sections; `--level beginner` omits the expert section and
  `--level expert` omits the beginner section; `tam search lua` resolves to
  `ecosystem/lua.html`; no literal entities leak into rendered text.
- Landed as commit `a6a97bb` on `develop`; no version bump.
