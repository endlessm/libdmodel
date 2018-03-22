eos-knowledge-content
=====================

This is Endless' C library for accessing knowledge application content. Through
gobject introspection it can be used in a wide variety of languages.

Building
--------

Normal build:

```bash
mkdir _build
cd _build
meson ..
ninja
ninja test
ninja install
```

Build documentation:

```bash
meson .. -Ddocumentation=true`
```

Coverage:

```bash
meson .. -Db_coverage=true
ninja
ninja test
ninja coverage-html
```

Static analysis:

```bash
ninja scan-build
```

```bash
meson .. -Db_sanitize=address,undefined
```

