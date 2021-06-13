import markdown
from markdown import treeprocessors
import os.path


class MarkdownCollector(treeprocessors.Treeprocessor):
    def __init__(self, md, code_refs: dict, base_dir):
        super().__init__(md)
        self.types = code_refs
        self.base_dir = base_dir

    def run(self, root):
        for element in root.iter():
            if element.tag == "a":
                found_link = None
                found_type = None
                for sub in element.iter():
                    if sub.tag == "a":
                        found_link = sub.attrib["href"]
                    if sub.tag == "code":
                        found_type = sub.text

                if found_link and found_type:
                    self.types[found_type] = os.path.join(self.base_dir[0], found_link)

        return None


class CollectorExtension(markdown.extensions.Extension):
    def __init__(self):
        super().__init__()
        self.code_refs = {}
        self.base_dir = [""]

    def extendMarkdown(self, md: markdown.Markdown) -> None:
        md.treeprocessors.register(MarkdownCollector(md, self.code_refs, self.base_dir), 'extract_links', 8)
        md.registerExtension(self)

    def set_base_dir(self, base_dir):
        self.base_dir[0] = base_dir


class Collector:
    def __init__(self):
        self.collector = CollectorExtension()
        self.parser = markdown.Markdown(extensions=[self.collector])

    def collect(self, base_dir, content):
        self.collector.set_base_dir(base_dir)
        self.parser.convert(content)

    def get_references(self):
        return self.collector.code_refs
