pandoc -s -o book.pdf book.md
pandoc -s --reference-doc=reference.docx -o book.docx book.md
open book.docx
open book.pdf
