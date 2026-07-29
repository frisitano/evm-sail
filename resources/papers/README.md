# Compiler-specialization papers

This directory contains research references for the evm-sail specialization
work. These papers are background material, not normative documentation for the
compiler.

Each included paper has its source PDF beside a Mathpix-generated Markdown
conversion. The PDF is the source of truth: equations, figures, tables, page
references, and code should be checked against it before being relied on in an
implementation or citation.

## Included papers

### Better Defunctionalization through Lambda Set Specialization

- Authors: William Brandon, Benjamin Driscoll, Frank Dai, Wilson Berkow, and
  Mae Milano
- Publication: PACMPL 7 (PLDI), 2023
- [PDF](better-defunctionalization-through-lambda-set-specialization/paper.pdf)
  and [Markdown](better-defunctionalization-through-lambda-set-specialization/paper.md)
- Original record: [author publication page](https://www.languagesforsyste.ms/publication/lss/)
  and [DOI 10.1145/3591260](https://doi.org/10.1145/3591260)
- License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), as stated
  on the paper's first page
- PDF provenance: user-provided source PDF; SHA-256
  `20b22288192d3386841de3c9d9d34d1fdb8842ba507307d4e61a8b6794ce56d6`
- Conversion provenance: Mathpix via Atlas Oracle (`mpxpy` backend), completed
  2026-07-29; the Markdown is a format conversion and may contain OCR or layout
  errors

### Webs and Flow-Directed Well-Typedness Preserving Program Transformations

- Authors: Benjamin Quiring, David Van Horn, John Reppy, and Olin Shivers
- Publication: PACMPL 9 (PLDI), 2025
- [PDF](webs-flow-directed-transformations/paper.pdf) and
  [Markdown](webs-flow-directed-transformations/paper.md)
- Original: [author-hosted PDF](https://bquiring.github.io/pdfs/Webs.pdf) and
  [DOI 10.1145/3729280](https://doi.org/10.1145/3729280)
- License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), as stated
  on the paper's first page
- PDF provenance: author-hosted PDF; SHA-256
  `9e6cbe4dd9f38c0735d087b3c8d950915f3f2564e446a0cc43064d39a1b60ff5`
- Conversion provenance: Mathpix via Atlas Oracle (`mpxpy` backend), completed
  2026-07-29; the Markdown is a format conversion and may contain OCR or layout
  errors

### Modularity, Code Specialization, and Zero-Cost Abstractions for Program Verification

- Authors: Son Ho, Aymeric Fromherz, and Jonathan Protzenko
- Publication: PACMPL 7 (ICFP), 2023
- [PDF](modularity-code-specialization-zero-cost/paper.pdf) and
  [Markdown](modularity-code-specialization-zero-cost/paper.md)
- Original: [arXiv 2102.01644v3](https://arxiv.org/abs/2102.01644) and
  [DOI 10.1145/3607844](https://doi.org/10.1145/3607844)
- License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), linked
  from the arXiv record
- PDF provenance: arXiv v3 PDF; SHA-256
  `473dec09755c4b5d0d046d882bffb458e8af6c75fbca94c2329e54c84f72b424`
- Conversion provenance: Mathpix via Atlas Oracle (`mpxpy` backend), completed
  2026-07-29; the Markdown is a format conversion and may contain OCR or layout
  errors

## Supplementary material

These papers are relevant to the work, but are linked rather than copied or
converted here because their available licensing does not clearly authorize
both repository redistribution and a derivative Markdown conversion.

- [Bound Analysis for Whiley Programs](https://research.usc.edu.au/esploro/outputs/journalArticle/Bound-Analysis-for-Whiley-Programs/99449149302621)
  — Min-Hsien Weng, Mark Utting, and Bernhard Pfahringer, 2016. The repository
  record marks it CC BY-NC-ND 4.0; the no-derivatives condition makes a Markdown
  conversion unsuitable for redistribution.
- [Bitwidth Analysis with Application to Silicon Compilation](https://groups.csail.mit.edu/cag/bitwise/)
  — Mark Stephenson, Jonathan Babb, and Saman Amarasinghe, 2000. The available
  ACM notice reserves server posting and redistribution absent permission.
- [TIL: A Type-Directed Optimizing Compiler for ML](https://www.microsoft.com/en-us/research/publication/til-a-type-directed-optimizing-compiler-for-ml/)
  — David Tarditi, Greg Morrisett, Perry Cheng, Chris Stone, Robert Harper, and
  Peter Lee, 1996. The publication page carries an ACM permission notice rather
  than an open-content license.
- [Practical Range Refinement Types with Inference](https://arxiv.org/abs/2607.00824)
  — Valentin Aebi and Carlo A. Furia, 2026. Its arXiv record uses the
  non-exclusive distribution license granted to arXiv, not a general license
  for third-party redistribution or derivative conversions.

## Copyright and attribution

The PDFs and their converted contents are third-party works and are not
relicensed by the evm-sail repository's software license. The three included
works are distributed under CC BY 4.0. Their authors, original records, license,
and the fact of conversion are identified above and in each Markdown file.
No author endorsement is implied.

CC BY 4.0 permits sharing and adaptation, including commercially, provided
appropriate credit is given, the license is linked, and changes are indicated.
Keep this README and each paper's conversion notice with redistributed copies.
