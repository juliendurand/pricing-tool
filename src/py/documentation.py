import os
import subprocess
import sys

import markdown
import numpy as np
import jinja2

import config
import result


def euro(n):
    return '{:,.0f}'.format(n).replace(',', ' ') + " €"


def thsep(n):
    return '{:,.0f}'.format(n).replace(',', ' ') + '&nbsp;&nbsp;&nbsp;'


def percent(p):
    return '{:,.2f}'.format(p * 100) + '%'


class Documentation:

    def __init__(self, config):
        self.path = config.get_doc_path()
        self.filename = config.get_doc_filename()
        self.result = result.Result(config)

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
        self.jinja2_env.filters['euro'] = euro
        self.jinja2_env.filters['thsep'] = thsep
        self.jinja2_env.filters['percent'] = percent

        img_path = os.path.join(self.path, 'img')
        if not os.path.exists(img_path):
            os.makedirs(img_path)

        self.result.write_coeffs_as_csv()

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

    def create_md(self):
        context = self.get_metadata()
        context['path'] = self.path
        context['result'] = self.result
        context['np'] = np
        md_content = self.jinja2_env.get_template('template.md') \
                         .render(context)
        return self.md.convert(md_content)

    def create_html(self, md):
        return self.jinja2_env.get_template('template.html') \
                   .render(content=md)

    def save_html(self):
        md = self.create_md()
        html = self.create_html(md)
        with open(self.filename, 'w') as out_file:
            out_file.write(html)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise Exception("Invalid number of options, expecting only one : "
                        "[config filename].")
    config_filename = sys.argv[1]
    configuration = config.Config(config_filename)

    print("Generating Documentation...")
    doc = Documentation(configuration)
    doc.save_html()
    subprocess.run('open ' + configuration.get_doc_filename(), shell=True)
    print("Document Finished\n")
