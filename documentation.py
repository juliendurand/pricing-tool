import os

import markdown
import jinja2


class Documentation:

    def __init__(self, path, name, results):
        self.path = path
        self.name = name
        self.results = results
        extensions = [
            'extra',
            'nl2br',
            'toc',
            'smarty',
            'meta',
        ]
        self.md = markdown.Markdown(extensions=extensions)
        jinja2_loader = jinja2.FileSystemLoader('./templates/')
        self.jinja2_env = jinja2.Environment(loader=jinja2_loader)

    def get_metadata(self):
        md_template = ''
        with open('./templates/template.md', 'r') as md_file:
            md_template = md_file.read()
        self.md.convert(md_template)
        context = self.md.Meta
        for k in context:
            if len(context[k]) == 1:
                context[k] = context[k][0]
        return context

    def create_html(self):
        context = self.get_metadata()
        context['path'] = self.path
        context['results'] = self.results
        md_content = self.jinja2_env.get_template('template.md') \
            .render(context)
        html_content = self.md.convert(md_content)
        html = self.jinja2_env.get_template('template.html') \
            .render(content=html_content)
        return html

    def save_html(self):
        html = self.create_html()
        if not os.path.exists(self.path):
            os.makedirs(self.path)
        filename = os.path.join(self.path, self.name + '.html')
        with open(filename, 'w') as out_file:
            out_file.write(html)
