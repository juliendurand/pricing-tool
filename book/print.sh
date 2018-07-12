pandoc --template template.latex -s -o book.pdf book.md && open book.pdf
pandoc -s --reference-doc=reference.docx -o book.docx book.md
