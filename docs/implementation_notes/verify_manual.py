"""Read-only structural and pagination checks for the implementation manual."""
from pathlib import Path
import hashlib
import json
import re
import zipfile
from docx import Document
from docx.oxml.ns import qn
from pypdf import PdfReader
import pdfplumber

work = Path('C:/Users/Administrator/.codex/visualizations/2026/10/09/01a12173-52ea-7e52-979d-1bc5d532652f/implementation_manual_work')
manifest = json.loads((work / 'manual_manifest.json').read_text(encoding='utf-8'))
doc_path = Path(manifest['output'])
render = work / 'render_v4'
pdf_path = render / (doc_path.stem + '.pdf')
with zipfile.ZipFile(doc_path) as package:
    assert package.testzip() is None
doc = Document(doc_path)
pages = PdfReader(pdf_path).pages
pngs = sorted(render.glob('page-*.png'))
assert len(pngs) == len(pages)
anchors = {b.get(qn('w:name')) for b in doc.element.iter(qn('w:bookmarkStart'))}
assert all(h['anchor'] in anchors for h in manifest['headings'])
links = list(doc.element.iter(qn('w:hyperlink')))
assert all(link.get(qn('w:anchor')) in anchors for link in links)
normalize = lambda value: re.sub(r'[\s\u200b\ufeff]+', '', value)
text = [normalize(page.extract_text() or '') for page in pages]
rendered_pages = {}
for heading in manifest['headings']:
    matches = [i + 1 for i, page in enumerate(text) if i >= 4 and normalize(heading['title']) in page]
    assert matches, heading['title']
    rendered_pages[heading['anchor']] = matches[0]
fields = 0
for field in doc.element.iter(qn('w:fldSimple')):
    match = re.match(r'PAGEREF\s+(\S+)', field.get(qn('w:instr'), ''))
    if not match:
        continue
    cached = ''.join(t.text or '' for t in field.iter(qn('w:t')))
    assert cached == str(rendered_pages[match.group(1)]), (match.group(1), cached)
    fields += 1
assert fields == len(manifest['headings'])
for table in doc.tables:
    assert table.rows[0]._tr.find('.//' + qn('w:tblHeader')) is not None
    assert all(row._tr.find('.//' + qn('w:cantSplit')) is not None for row in table.rows)
changed = []
identical = []
for page in range(1, len(pages) + 1):
    current = render / f'page-{page}.png'
    old = work / 'render_v3' / current.name
    target = identical if old.exists() and hashlib.sha256(old.read_bytes()).digest() == hashlib.sha256(current.read_bytes()).digest() else changed
    target.append(page)
outside = []
with pdfplumber.open(pdf_path) as pdf:
    for number, page in enumerate(pdf.pages, 1):
        for char in page.chars:
            if char['x0'] < 0 or char['x1'] > page.width + .1 or char['top'] < 0 or char['bottom'] > page.height + .1:
                outside.append(number)
                break
assert not outside, outside
result = {'pages': len(pages), 'linked_toc_entries': fields, 'tables': len(doc.tables), 'figures': len(doc.inline_shapes), 'changed_png_pages': changed, 'identical_png_pages': identical, 'out_of_page_text': outside, 'docx_sha256': hashlib.sha256(doc_path.read_bytes()).hexdigest()}
(work / 'structural_qa.json').write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
print(json.dumps(result))
