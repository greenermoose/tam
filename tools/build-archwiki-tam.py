#!/usr/bin/env python3
"""Pre-process arch-wiki-docs and arch-wiki-lite into compact tam-pager HTML.

Takes arch-wiki-docs (archive or html/en directory) and arch-wiki-lite
(archive or text directory), applies any curated link-mend patches from
patches/arch-wiki-docs/, strips HTML tags unsupported by tam's pager,
resolves/repairs internal wiki links, rewrites unresolvable links to
tam:archwiki:broken?target=...&from=..., and writes the result to
/usr/share/doc/arch-wiki/tam/ (or --output).
"""

from __future__ import annotations

import argparse
import difflib
import gzip
import html
from html.parser import HTMLParser
import json
import os
from pathlib import Path, PurePosixPath
import posixpath
import re
import shutil
import subprocess
import tarfile
import tempfile
import urllib.parse


ALLOWED_VOID_TAGS = {"br", "hr"}
ALLOWED_BLOCK_TAGS = {
    "h1", "h2", "h3", "h4", "h5", "h6",
    "p", "blockquote", "pre",
    "ul", "ol", "li", "dl", "dt", "dd",
    "table", "thead", "tbody", "tfoot", "tr", "th", "td",
    "aside",
}
ALLOWED_INLINE_TAGS = {"code", "strong", "b", "em", "i", "dfn", "a"}
SKIP_SUBTREE_TAGS = {"script", "style", "svg", "button", "figure", "figcaption", "nav", "noscript"}
END_MARKERS = (
    '<div id="catlinks"',
    '<div class="printfooter"',
    '<footer',
    '</main>',
    '</body>',
)
HEX_HASH_RE = re.compile(r"^[0-9a-f]{32}$")


def is_archive(path: Path) -> bool:
    name = path.name
    return path.is_file() and (
        name.endswith(".pkg.tar.zst")
        or name.endswith(".pkg.tar.xz")
        or name.endswith(".pkg.tar.gz")
        or name.endswith(".tar.zst")
        or name.endswith(".tar.gz")
        or name.endswith(".tar.xz")
        or name.endswith(".tar")
    )


def extract_archive_prefix(archive: Path, prefix: str, dest_dir: Path) -> int:
    """Extract files under `prefix` from a pacman .pkg.tar.* archive safely."""
    count = 0
    fd = os.open(archive, os.O_RDONLY | os.O_NOFOLLOW)
    with os.fdopen(fd, "rb") as stream:
        proc = subprocess.Popen(
            ["/usr/bin/bsdtar", "-cf", "-", "@/proc/self/fd/" + str(stream.fileno())],
            pass_fds=(stream.fileno(),),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        try:
            with tarfile.open(fileobj=proc.stdout, mode="r|") as tar:
                for member in tar:
                    name = member.name.removeprefix("./")
                    if not name.startswith(prefix):
                        continue
                    rel = PurePosixPath(name[len(prefix):])
                    if not str(rel) or str(rel) == "." or rel.is_absolute() or ".." in rel.parts:
                        continue
                    if member.isdir():
                        continue
                    if not member.isfile():
                        continue
                    target = dest_dir / rel
                    target.parent.mkdir(parents=True, exist_ok=True)
                    with tar.extractfile(member) as incoming, target.open("wb") as out:
                        shutil.copyfileobj(incoming, out)
                    count += 1
            if proc.wait() != 0:
                err = proc.stderr.read().decode("utf-8", errors="replace")
                raise RuntimeError(f"bsdtar failed on {archive}: {err}")
        finally:
            if proc.stdout:
                proc.stdout.close()
            if proc.stderr:
                proc.stderr.close()
    return count


def load_archwiki_lite(lite_path: Path, scratch: Path) -> dict[str, list[str]]:
    """Load arch-wiki-lite English index and per-page text lines as structural guide."""
    if is_archive(lite_path):
        lite_dir = scratch / "lite"
        lite_dir.mkdir(parents=True, exist_ok=True)
        extract_archive_prefix(lite_path, "usr/share/doc/arch-wiki/text/", lite_dir)
    else:
        lite_dir = lite_path
        if (lite_dir / "usr/share/doc/arch-wiki/text").is_dir():
            lite_dir = lite_dir / "usr/share/doc/arch-wiki/text"

    index_file = lite_dir / "index"
    text_gz = lite_dir / "arch-wiki.txt.gz"
    text_plain = lite_dir / "arch-wiki.txt"

    if not index_file.is_file():
        return {}

    id_to_slug: dict[str, str] = {}
    for line in index_file.read_text(encoding="utf-8", errors="replace").splitlines():
        parts = line.rsplit(" ", 1)
        if len(parts) != 2:
            continue
        key, doc_id = parts
        if key.startswith("en/"):
            id_to_slug[doc_id] = key[3:]

    pages: dict[str, list[str]] = {slug: [] for slug in id_to_slug.values()}

    if text_gz.is_file():
        opener = lambda: gzip.open(text_gz, "rt", encoding="utf-8", errors="replace")
    elif text_plain.is_file():
        opener = lambda: text_plain.open("rt", encoding="utf-8", errors="replace")
    else:
        return pages

    with opener() as fh:
        for raw_line in fh:
            if ":" not in raw_line:
                continue
            doc_id, content = raw_line.split(":", 1)
            slug = id_to_slug.get(doc_id)
            if slug is None:
                continue
            cleaned = content.replace("@@b", "").replace("@@w", "").replace("@@u", "").strip()
            if cleaned:
                pages[slug].append(cleaned)
    return pages


def apply_patches(en_dir: Path, patches_dir: Path | None) -> list[str]:
    """Apply sorted *.patch unified diffs in patches_dir to en_dir."""
    if patches_dir is None or not patches_dir.is_dir():
        return []
    applied = []
    for patch_file in sorted(patches_dir.glob("*.patch")):
        for strip in ("-p1", "-p0"):
            res = subprocess.run(
                ["patch", strip, "--batch", "--forward", "-i", str(patch_file)],
                cwd=en_dir,
                capture_output=True,
                text=True,
            )
            if res.returncode == 0:
                applied.append(patch_file.name)
                break
        else:
            raise RuntimeError(f"Failed to apply patch {patch_file.name}: {res.stderr or res.stdout}")
    return applied


def extract_title_and_body(raw_html: str, fallback_slug: str) -> tuple[str, str]:
    m = re.search(r"<title[^>]*>(.*?)</title>", raw_html, flags=re.IGNORECASE | re.DOTALL)
    if m:
        title = html.unescape(m.group(1)).strip()
        if title.endswith(" - ArchWiki"):
            title = title[: -len(" - ArchWiki")].strip()
    else:
        title = fallback_slug.replace("_", " ")

    start = 0
    for marker in ('<div class="mw-content-ltr mw-parser-output"', '<div id="mw-content-text"', "<body"):
        idx = raw_html.find(marker)
        if idx != -1:
            gt = raw_html.find(">", idx)
            if gt != -1:
                start = gt + 1
                break

    end = len(raw_html)
    for marker in END_MARKERS:
        idx = raw_html.find(marker, start)
        if idx != -1 and idx < end:
            end = idx

    return title, raw_html[start:end]


class TamHTMLSanitizer(HTMLParser):
    """Strip unsupported MediaWiki HTML and collect links + fragment IDs."""

    def __init__(self) -> None:
        super().__init__(convert_charrefs=False)
        self.out: list[str] = []
        self.skip_depth = 0
        self.skip_stack: list[str] = []
        self.div_stack: list[str | None] = []  # None or "aside"
        self.ids: set[str] = set()
        self.links: list[tuple[int, str, str]] = []  # (out_index_of_href, raw_href, title_attr)

    def _emit_attrs(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        attr_map = {k.lower(): (v or "") for k, v in attrs}
        if "id" in attr_map and attr_map["id"]:
            self.ids.add(attr_map["id"])
        if tag == "a" and "name" in attr_map and attr_map["name"]:
            self.ids.add(attr_map["name"])

        if tag in ("h1", "h2", "h3", "h4", "h5", "h6", "li", "dt"):
            if attr_map.get("id"):
                self.out.append(f' id="{html.escape(attr_map["id"], quote=True)}"')
        elif tag == "p":
            align = attr_map.get("align", "").lower()
            if align in ("left", "center", "right"):
                self.out.append(f' align="{align}"')
        elif tag in ("th", "td"):
            colspan = attr_map.get("colspan", "")
            if colspan.isdigit() and 1 <= int(colspan) <= 64:
                self.out.append(f' colspan="{colspan}"')
        elif tag == "a":
            if attr_map.get("id"):
                self.out.append(f' id="{html.escape(attr_map["id"], quote=True)}"')
            if attr_map.get("name"):
                self.out.append(f' name="{html.escape(attr_map["name"], quote=True)}"')
            if "href" in attr_map:
                href_val = html.unescape(attr_map["href"])
                title_val = html.unescape(attr_map.get("title", ""))
                idx = len(self.out)
                self.out.append(f' href="{html.escape(href_val, quote=True)}"')
                self.links.append((idx, href_val, title_val))

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        t = tag.lower()
        if self.skip_depth > 0:
            if t not in ALLOWED_VOID_TAGS and t not in ("img", "meta", "link", "input"):
                self.skip_depth += 1
                self.skip_stack.append(t)
            return

        attr_map = {k.lower(): (v or "") for k, v in attrs}
        cls = attr_map.get("class", "")
        cls_tokens = set(cls.split())
        elem_id = attr_map.get("id", "")

        if t in SKIP_SUBTREE_TAGS:
            self.skip_depth = 1
            self.skip_stack.append(t)
            return

        if (
            "mw-editsection" in cls_tokens
            or "navbox" in cls_tokens
            or "mw-indicators" in cls_tokens
            or "printfooter" in cls_tokens
            or elem_id in ("toc", "catlinks")
        ):
            self.skip_depth = 1
            self.skip_stack.append(t)
            return

        if t == "div":
            if "archwiki-template-box" in cls_tokens:
                if "archwiki-template-box-warning" in cls_tokens:
                    aside_cls = "caution"
                elif "archwiki-template-box-tip" in cls_tokens:
                    aside_cls = "tip"
                else:
                    aside_cls = "note"
                self.div_stack.append("aside")
                self.out.append(f'<aside class="{aside_cls}">')
                return
            self.div_stack.append(None)
            if elem_id:
                self.ids.add(elem_id)
                self.out.append(f'<a id="{html.escape(elem_id, quote=True)}"></a>')
            return

        if t == "span":
            if elem_id:
                self.ids.add(elem_id)
                self.out.append(f'<a id="{html.escape(elem_id, quote=True)}"></a>')
            return

        if t in ALLOWED_VOID_TAGS:
            self.out.append(f"<{t}>")
            return

        if t in ALLOWED_BLOCK_TAGS or t in ALLOWED_INLINE_TAGS:
            self.out.append(f"<{t}")
            self._emit_attrs(t, attrs)
            self.out.append(">")
            return

        # Any other unknown tag is unwrapped, preserving its id anchor if present.
        if elem_id:
            self.ids.add(elem_id)
            self.out.append(f'<a id="{html.escape(elem_id, quote=True)}"></a>')

    def handle_endtag(self, tag: str) -> None:
        t = tag.lower()
        if self.skip_depth > 0:
            if self.skip_stack and self.skip_stack[-1] == t:
                self.skip_stack.pop()
                self.skip_depth -= 1
            elif t in self.skip_stack:
                while self.skip_stack:
                    popped = self.skip_stack.pop()
                    self.skip_depth -= 1
                    if popped == t:
                        break
            return

        if t == "div":
            if self.div_stack:
                kind = self.div_stack.pop()
                if kind == "aside":
                    self.out.append("</aside>")
            return

        if t in ALLOWED_BLOCK_TAGS or t in ALLOWED_INLINE_TAGS:
            self.out.append(f"</{t}>")

    def handle_data(self, data: str) -> None:
        if self.skip_depth == 0:
            self.out.append(data)

    def handle_entityref(self, name: str) -> None:
        if self.skip_depth == 0:
            self.out.append(f"&{name};")

    def handle_charref(self, name: str) -> None:
        if self.skip_depth == 0:
            self.out.append(f"&#{name};")


def normalize_punct(text: str) -> str:
    return re.sub(r"[^a-z0-9/]", "", text.lower())


class LinkResolver:
    """Resolve and mend internal Arch Wiki links across all English pages."""

    def __init__(self, pages: dict[str, set[str]]) -> None:
        self.pages = pages  # canonical slug -> set of element IDs
        self.all_slugs = sorted(pages.keys())
        self.lower_map: dict[str, str] = {}
        self.punct_map: dict[str, str] = {}
        self.base_map: dict[str, list[str]] = {}
        self.id_lower_map: dict[str, dict[str, str]] = {}

        for slug, ids in pages.items():
            self.lower_map.setdefault(slug.lower(), slug)
            self.punct_map.setdefault(normalize_punct(slug), slug)
            base = slug.rsplit("/", 1)[-1].lower()
            self.base_map.setdefault(base, []).append(slug)
            self.id_lower_map[slug] = {i.lower(): i for i in ids}

    def _resolve_fragment(self, target_slug: str, frag: str) -> str:
        if not frag:
            return ""
        ids = self.pages.get(target_slug, set())
        if frag in ids:
            return "#" + frag
        low = self.id_lower_map.get(target_slug, {}).get(frag.lower())
        if low:
            return "#" + low
        return "#" + frag

    def resolve(
        self, source_slug: str, raw_href: str, title_attr: str = ""
    ) -> tuple[str, str | None]:
        """Return (rewritten_href, status) where status is None|'repaired'|'broken'."""
        if not raw_href:
            return raw_href, None

        if raw_href.startswith(("http://", "https://", "mailto:", "irc://", "man:", "tam:", "info:")):
            return raw_href, None

        if raw_href.startswith("#"):
            return self._resolve_fragment(source_slug, raw_href[1:]), None

        # Split fragment
        if "#" in raw_href:
            path_part, frag = raw_href.split("#", 1)
        else:
            path_part, frag = raw_href, ""

        path_part = urllib.parse.unquote(path_part).strip()
        if not path_part:
            return self._resolve_fragment(source_slug, frag), None

        # Non-article / special / external-to-en paths
        if path_part.startswith("/") or path_part.startswith(("Special:", "File:", "MediaWiki:")):
            return (
                f"tam:archwiki:broken?target={urllib.parse.quote(raw_href, safe='')}"
                f"&from={urllib.parse.quote(source_slug, safe='')}",
                "broken",
            )

        # Resolve relative path against source_slug's directory
        source_dir = source_slug.rsplit("/", 1)[0] if "/" in source_slug else ""
        # Strip leading ../en/ or ../../en/ etc.
        m = re.match(r"^(?:\.\./)+en/(.+)$", path_part)
        if m:
            cand = m.group(1)
        else:
            joined = f"{source_dir}/{path_part}" if source_dir else path_part
            cand = posixpath.normpath(joined)
            if cand.startswith("en/"):
                cand = cand[3:]

        if cand.endswith(".html"):
            cand = cand[:-5]
        cand = cand.replace(" ", "_").strip("/")

        # 1. Exact match
        if cand in self.pages:
            return f"archwiki:{cand}{self._resolve_fragment(cand, frag)}", None

        # 2. Case-insensitive match
        if cand.lower() in self.lower_map:
            target = self.lower_map[cand.lower()]
            return f"archwiki:{target}{self._resolve_fragment(target, frag)}", "repaired"

        # 3. Title attribute match
        if title_attr:
            tcand = title_attr.strip().replace(" ", "_")
            if tcand in self.pages:
                return f"archwiki:{tcand}{self._resolve_fragment(tcand, frag)}", "repaired"
            if tcand.lower() in self.lower_map:
                target = self.lower_map[tcand.lower()]
                return f"archwiki:{target}{self._resolve_fragment(target, frag)}", "repaired"

        # 4. Normalized punctuation match
        pnorm = normalize_punct(cand)
        if pnorm in self.punct_map:
            target = self.punct_map[pnorm]
            return f"archwiki:{target}{self._resolve_fragment(target, frag)}", "repaired"

        # 5. Unique basename match
        base = cand.rsplit("/", 1)[-1].lower()
        if base in self.base_map and len(self.base_map[base]) == 1:
            target = self.base_map[base][0]
            return f"archwiki:{target}{self._resolve_fragment(target, frag)}", "repaired"

        # 6. High-confidence fuzzy match
        matches = difflib.get_close_matches(cand, self.all_slugs, n=1, cutoff=0.88)
        if matches:
            target = matches[0]
            return f"archwiki:{target}{self._resolve_fragment(target, frag)}", "repaired"

        # 7. Unresolvable -> broken-page link
        broken_href = (
            f"tam:archwiki:broken?target={urllib.parse.quote(raw_href, safe='')}"
            f"&from={urllib.parse.quote(source_slug, safe='')}"
        )
        return broken_href, "broken"


def find_en_dir(docs_path: Path, scratch: Path) -> Path:
    if is_archive(docs_path):
        en_dir = scratch / "docs/en"
        en_dir.mkdir(parents=True, exist_ok=True)
        count = extract_archive_prefix(docs_path, "usr/share/doc/arch-wiki/html/en/", en_dir)
        if count == 0:
            raise RuntimeError(f"No usr/share/doc/arch-wiki/html/en/ files found in {docs_path}")
        return en_dir

    if (docs_path / "usr/share/doc/arch-wiki/html/en").is_dir():
        src = docs_path / "usr/share/doc/arch-wiki/html/en"
    elif (docs_path / "html/en").is_dir():
        src = docs_path / "html/en"
    else:
        src = docs_path

    # Copy to scratch if we might apply patches without mutating input dir
    en_dir = scratch / "docs/en"
    shutil.copytree(src, en_dir)
    return en_dir


def process_archwiki(
    docs_path: Path,
    lite_path: Path | None,
    output_dir: Path,
    patches_dir: Path | None = None,
    report_path: Path | None = None,
) -> dict:
    output_dir.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory(prefix="archwiki-tam-") as tmp:
        scratch = Path(tmp)
        en_dir = find_en_dir(docs_path, scratch)
        applied_patches = apply_patches(en_dir, patches_dir)
        lite_pages = load_archwiki_lite(lite_path, scratch) if lite_path else {}

        # Pass 1: sanitize HTML and collect all page slugs + anchor IDs
        parsed: dict[str, tuple[str, TamHTMLSanitizer]] = {}
        page_ids: dict[str, set[str]] = {}

        for html_file in sorted(en_dir.rglob("*.html")):
            rel = html_file.relative_to(en_dir).as_posix()
            slug = rel[:-5]
            if "/" not in slug and HEX_HASH_RE.match(slug):
                continue
            raw_html = html_file.read_text(encoding="utf-8", errors="replace")
            title, body = extract_title_and_body(raw_html, slug)
            san = TamHTMLSanitizer()
            san.feed(body)
            san.close()
            parsed[slug] = (title, san)
            page_ids[slug] = san.ids

        resolver = LinkResolver(page_ids)
        broken_links: list[dict[str, str]] = []
        repaired_links: list[dict[str, str]] = []
        total_bytes = 0
        lite_guided = 0

        # Pass 2: resolve/mend links and write compact tam HTML
        for slug, (title, san) in parsed.items():
            if slug in lite_pages:
                lite_guided += 1
            tokens = list(san.out)
            for idx, raw_href, title_attr in san.links:
                new_href, status = resolver.resolve(slug, raw_href, title_attr)
                tokens[idx] = f' href="{html.escape(new_href, quote=True)}"'
                if status == "broken":
                    broken_links.append({"from": slug, "href": raw_href, "title": title_attr})
                elif status == "repaired":
                    repaired_links.append({
                        "from": slug,
                        "href": raw_href,
                        "repaired": new_href,
                        "title": title_attr,
                    })

            body_html = re.sub(r"\n{3,}", "\n\n", "".join(tokens)).strip()
            out_doc = (
                "<!DOCTYPE html>\n<html>\n<head>\n"
                "<meta charset=\"utf-8\">\n"
                f"<title>{html.escape(title)}</title>\n"
                "</head>\n<body>\n"
                f"{body_html}\n"
                "</body>\n</html>\n"
            )
            dest = output_dir / f"{slug}.html"
            dest.parent.mkdir(parents=True, exist_ok=True)
            encoded = out_doc.encode("utf-8")
            dest.write_bytes(encoded)
            total_bytes += len(encoded)

    summary = {
        "pages": len(parsed),
        "lite_guided_pages": lite_guided,
        "total_bytes": total_bytes,
        "applied_patches": applied_patches,
        "repaired_count": len(repaired_links),
        "broken_count": len(broken_links),
    }

    broken_file = report_path if report_path else output_dir / "broken-links.json"
    broken_file.write_text(
        json.dumps({"summary": summary, "broken": broken_links}, indent=2) + "\n",
        encoding="utf-8",
    )
    (output_dir / "repaired-links.json").write_text(
        json.dumps(repaired_links, indent=2) + "\n",
        encoding="utf-8",
    )
    return summary


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--docs", required=True, type=Path, help="arch-wiki-docs archive or html/en dir")
    parser.add_argument("--lite", type=Path, default=None, help="arch-wiki-lite archive or text dir")
    parser.add_argument("--output", default=Path("/usr/share/doc/arch-wiki/tam"), type=Path)
    parser.add_argument(
        "--patches-dir",
        default=Path(__file__).resolve().parents[1] / "patches/arch-wiki-docs",
        type=Path,
    )
    parser.add_argument("--report", type=Path, default=None, help="Optional path for broken-links.json")
    args = parser.parse_args()

    summary = process_archwiki(
        docs_path=args.docs,
        lite_path=args.lite,
        output_dir=args.output,
        patches_dir=args.patches_dir,
        report_path=args.report,
    )
    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
