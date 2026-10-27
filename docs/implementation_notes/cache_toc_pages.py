"""Fill Word's PAGEREF fallback text from the rendered manual's pagination."""
from pathlib import Path
import json
import re
import sys
from docx import Document
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from pypdf import PdfReader

work = Path(sys.argv[1])
pdf_path = Path(sys.argv[2])
manifest = json.loads((work / 'manual_manifest.json').read_text(encoding='utf-8'))
pages = PdfReader(pdf_path).pages

def normalized(value):
    return re.sub(r'[\s\u200b\ufeff]+', '', value)

page_text = [normalized(page.extract_text() or '') for page in pages]
mapping = {}
for heading in manifest['headings']:
    title = normalized(heading['title'])
    matches = [index + 1 for index, body in enumerate(page_text) if index >= 4 and title in body]
    if not matches:
        raise ValueError(f"No rendered body heading found for {heading['title']}")
    mapping[heading['anchor']] = matches[0]

doc = Document(manifest['output'])
count = 0
for field in doc.element.iter(qn('w:fldSimple')):
    instruction = field.get(qn('w:instr'), '')
    match = re.match(r'PAGEREF\s+(\S+)', instruction)
    if not match:
        continue
    texts = list(field.iter(qn('w:t')))
    if len(texts) != 1:
        raise ValueError(f'Unexpected field structure for {instruction}')
    texts[0].text = str(mapping[match.group(1)])
    count += 1
setting = doc.settings.element.find(qn('w:updateFields'))
if setting is None:
    setting = OxmlElement('w:updateFields')
    doc.settings.element.append(setting)
setting.set(qn('w:val'), 'true')
doc.save(manifest['output'])
(work / 'toc_pages.json').write_text(json.dumps(mapping, ensure_ascii=False, indent=2), encoding='utf-8')
print(json.dumps({'cached_fields': count, 'pages': len(pages)}))
