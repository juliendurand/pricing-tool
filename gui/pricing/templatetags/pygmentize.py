import pygments
from django import template
from django.utils.safestring import mark_safe
from pygments import lexers
# from pygments import formatters
from pygments.formatters import HtmlFormatter

register = template.Library()


@register.filter(name='pygmentize', is_safe=True)
def pygmentize(value, language):
    lexer = lexers.get_lexer_by_name(language)
    output = pygments.highlight(value, lexer, HtmlFormatter())
    #print(HtmlFormatter().get_style_defs('.highlight'))
    return mark_safe(output)

#@register.filter(name='pygmentize_css', is_safe=True)
#def pygmentize_css(value, language):
#    output = HtmlFormatter().get_style_defs('.highlight'))
#    return mark_safe(output)
