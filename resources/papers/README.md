# Research papers

This directory contains research references for the evm-sail compiler
specialization and Iris/Islaris verification work. These papers are background
material, not normative documentation for the compiler.

Each included paper has its source PDF beside a Mathpix-generated Markdown
conversion. The PDF is the source of truth: equations, figures, tables, page
references, and code should be checked against it before being relied on in an
implementation or citation.

## Compiler specialization

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

## Iris, Islaris, and foundational verification

### Islaris: Verification of Machine Code Against Authoritative ISA Semantics

- Authors: Michael Sammler, Angus Hammond, Rodolphe Lepigre, Brian Campbell,
  Jean Pichon-Pharabod, Derek Dreyer, Deepak Garg, and Peter Sewell
- Publication: PLDI, 2022
- [PDF](islaris-verification-machine-code/paper.pdf) and
  [Markdown](islaris-verification-machine-code/paper.md)
- Original: [author-hosted PDF](https://www.cl.cam.ac.uk/~jp622/islaris.pdf)
  and [DOI 10.1145/3519939.3523434](https://doi.org/10.1145/3519939.3523434)
- License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), as stated
  on the paper's first page
- PDF provenance: author-hosted PDF; SHA-256
  `ae93493acdb6a2239615b1b3b90aa0bd7f66ee5b80c9edc1d779fbe0d3e1ffba`
- Conversion provenance: Mathpix via Atlas Oracle (`mpxpy` backend), completed
  2026-07-30; headings, tables, footnotes, and extracted figures were manually
  normalized after conversion
- Relevance: Islaris connects Isla traces from authoritative Sail ISA semantics
  to an Iris separation logic and automated Coq proofs, making it the most
  direct verification reference for evm-sail

### Verified Symbolic Execution with Kripke Specification Monads (and No Meta-programming)

- Authors: Steven Keuchel, Sander Huyghebaert, Georgy Lukyanov, and Dominique
  Devriese
- Publication: PACMPL 6 (ICFP), Article 97, 2022
- [PDF](verified-symbolic-execution-kripke-monads/paper.pdf) and
  [Markdown](verified-symbolic-execution-kripke-monads/paper.md)
- Original:
  [Iris-hosted PDF](https://iris-project.org/pdfs/2022-icfp-symbexec-final.pdf)
  and [DOI 10.1145/3547628](https://doi.org/10.1145/3547628)
- License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), as stated
  on the paper's first page
- PDF provenance: Iris project-hosted PDF; SHA-256
  `6d75fb7fd1e3410c7ac881d934efa3b2fa6e98cec4e40d0ba7edf075561aad8d`
- Conversion provenance: Mathpix via Atlas Oracle (`mpxpy` backend), completed
  2026-07-30; headings, tables, footnotes, formulas, and extracted figures were
  manually normalized after conversion
- Relevance: this paper explains Katamaran's verified symbolic execution for
  µSail, its Kripke specification monads, and the Iris model connecting its
  program logic to an operational semantics

### RefinedC: Automating the Foundational Verification of C Code with Refined Ownership Types

- Authors: Michael Sammler, Rodolphe Lepigre, Robbert Krebbers, Kayvan
  Memarian, Derek Dreyer, and Deepak Garg
- Publication: PLDI, 2021
- [PDF](refinedc/paper.pdf) and [Markdown](refinedc/paper.md)
- Original: [project-hosted PDF](https://plv.mpi-sws.org/refinedc/paper.pdf)
  and [DOI 10.1145/3453483.3454036](https://doi.org/10.1145/3453483.3454036)
- License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), as stated
  on the paper's first page
- PDF provenance: RefinedC project-hosted PDF; SHA-256
  `80d43dea0e754222a6aa851c884b4d2aee59d288a0237409952e45941104c9e0`
- Conversion provenance: Mathpix via Atlas Oracle (`mpxpy` backend), completed
  2026-07-30; headings, tables, footnotes, code, formulas, and extracted figures
  were manually normalized after conversion
- Relevance: RefinedC introduces Lithium's predictable proof-search discipline
  on top of Iris; Islaris builds on this automation style for machine-code
  verification

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
relicensed by the evm-sail repository's software license. All six included
works are distributed under CC BY 4.0. Their authors, original records, license,
and the fact of conversion are identified above and in each Markdown file. No
author endorsement is implied.

CC BY 4.0 permits sharing and adaptation, including commercially, provided
appropriate credit is given, the license is linked, and changes are indicated.
Keep this README and each paper's conversion notice with redistributed copies.
